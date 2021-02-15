/* ----------------------------------------------------------------------
   LAMMPS - Large-scale Atomic/Molecular Massively Parallel Simulator
   http://lammps.sandia.gov, Sandia National Laboratories
   Steve Plimpton, sjplimp@sandia.gov

   Copyright (2003) Sandia Corporation.  Under the terms of Contract
   DE-AC04-94AL85000 with Sandia Corporation, the U.S. Government retains
   certain rights in this software.  This software is distributed under
   the GNU General Public License.

   See the README file in the top-level LAMMPS directory.
------------------------------------------------------------------------- */

#include "fix_mbx.h"
#include <mpi.h>
#include <cmath>
#include <cstring>
#include "pair_mbx.h"
#include "atom.h"
#include "comm.h"
#include "domain.h"
#include "neighbor.h"
#include "neigh_list.h"
#include "neigh_request.h"
#include "update.h"
#include "force.h"
#include "group.h"
#include "pair.h"
#include "respa.h"
#include "memory.h"
#include "citeme.h"
#include "error.h"
#include "universe.h"

#define _MAX_SIZE_MOL_NAME 10
#define SMALL 1.0e-4

//#define _DEBUG

using namespace LAMMPS_NS;
using namespace FixConst;

static const char cite_fix_mbx[] =
  "fix mbx command:\n\n"
  "@Article{,\n"
  " author = {},\n"
  " title = {},\n"
  " journal = {},\n"
  " year =    ,\n"
  " volume =  ,\n"
  " pages =   {}\n"
  "}\n\n";

/* ---------------------------------------------------------------------- */

FixMBX::FixMBX(LAMMPS *lmp, int narg, char **arg) :
  Fix(lmp, narg, arg)
{
  if (lmp->citeme) lmp->citeme->add(cite_fix_mbx);

  me = comm->me;
  nprocs = comm->nprocs;
  
  if (narg<5) error->all(FLERR,"Illegal fix mbx command");

  num_mol_types = utils::inumeric(FLERR,arg[3],false,lmp);
  
  if (num_mol_types < 1) error->all(FLERR,"Illegal fix mbx command");

  num_mols  = NULL;
  num_atoms_per_mol = NULL;
  mol_names = NULL;
  
  memory->create(num_mols, num_mol_types, "fixmbx:num_mols");
  memory->create(num_atoms_per_mol, num_mol_types, "fixmbx:num_atoms_per_mol");
  memory->create(mol_names, num_mol_types, _MAX_SIZE_MOL_NAME, "fixmbx:mol_names");

  int iarg = 4;
  for(int i=0; i<num_mol_types; ++i) {
    num_mols[i] = utils::inumeric(FLERR,arg[iarg++],false,lmp);
    strcpy(mol_names[i], arg[iarg++]);
  }
  
  // process remaining optional keywords
  
  use_json = 0;
  json_file = NULL;
  print_settings = 0;
  
  while(iarg < narg) {
    if(strcmp(arg[iarg], "json") == 0) {
      int len = strlen(arg[++iarg]);
      use_json = 1;
      json_file = new char[len];
      strcpy(json_file, arg[iarg]);
    }
    else if(strcmp(arg[iarg], "print/settings") == 0) {
      if(me == 0) print_settings = 1;
    }
    
    iarg++;
  }
  
  mol_offset = NULL;
  memory->create(mol_offset, num_mol_types+1, "fixmbx:mol_offset");

  // assign # of atoms per molecule based on molecule name
  // -- use this as first pass whether molecule supported by MBX

  for(int i=0; i<num_mol_types; ++i) {
    if(      strcmp("h2o", mol_names[i]) == 0) num_atoms_per_mol[i] = 3;
    else if (strcmp("na",  mol_names[i]) == 0) num_atoms_per_mol[i] = 1;
    else if (strcmp("cl",  mol_names[i]) == 0) num_atoms_per_mol[i] = 1;
    else if (strcmp("co2", mol_names[i]) == 0) num_atoms_per_mol[i] = 3;
    else if (strcmp("ch4", mol_names[i]) == 0) num_atoms_per_mol[i] = 5;
    else if (strcmp("he", mol_names[i]) == 0) num_atoms_per_mol[i] = 1;
    else error->all(FLERR,"Unsupported molecule type in MBX");
  }

  // check that total number of atoms matches what is expected

  int na = 0;
  for(int i=0; i<num_mol_types; ++i) na += num_mols[i] * num_atoms_per_mol[i];
  
  mol_offset[0] = 0;
  mol_offset[1] = num_mols[0]*num_atoms_per_mol[0];
  for(int i=1; i<num_mol_types; ++i)
    mol_offset[i+1] = mol_offset[i] + num_mols[i]*num_atoms_per_mol[i];

  num_molecules = 0;
  for(int i=0; i<num_mol_types; ++i) num_molecules += num_mols[i];

  if(screen && comm->me == 0) {
    if(use_json) fprintf(screen,"\n[MBX] Using json_file= %s\n",json_file);
    fprintf(screen,"[MBX] # molecule types= %i\n",num_mol_types);
    fprintf(screen,"[MBX] # molecules=      %i\n",num_molecules);
    for(int i=0; i<num_mol_types; ++i)
      fprintf(screen,"[MBX]   i= %i  # of molecules= %i  name= '%4s'  offset= %i\n",i,num_mols[i],mol_names[i],mol_offset[i]);
    fprintf(screen,"\n");
  }

  if(na != atom->natoms) error->all(FLERR,"Inconsistent # of atoms");

  mbx_mpi_enabled = true;
  
  pair_mbx = NULL;
  pair_mbx = (PairMBX *) force->pair_match("^mbx",0);

  ptr_mbx = NULL;
  
  mol_type = NULL;
  mol_anchor = NULL;
  mol_local = NULL;
  grow_arrays(atom->nmax);
  atom->add_callback(0);
  atom->add_callback(1);

  mbx_num_atoms = 0;
  
  // for terms not yet LAMMPS-parallelized
  
  ptr_mbx_full = NULL;
  mbx_num_atoms_full = 0;

  mol_anchor_full = NULL;
  mol_type_full = NULL;
  x_full = NULL;
  f_full = NULL;
  f_local = NULL;
  tag_full = NULL;
  atom_map_full = NULL;
  nlocal_rank = NULL;
  nlocal_disp = NULL;
  nlocal_rank3 = NULL;
  nlocal_disp3 = NULL;

  // instance of MBX with just local monomers

  ptr_mbx_local = NULL;
  mbx_num_atoms_local = 0;

  // check that LAMMPS proc mapping matches PME solver

  if(comm->style != 0) error->all(FLERR,"Fix mbx must be used with comm_style brick");

  if(comm->layout != Comm::LAYOUT_UNIFORM)
    error->all(FLERR,"Fix mbx must be used with comm layout of equal-sized bricks");
  
  {
    int proc_x = me % comm->procgrid[0];
    int proc_y = (me % (comm->procgrid[0] * comm->procgrid[1])) / comm->procgrid[0];
    int proc_z = me / (comm->procgrid[0] * comm->procgrid[1]);

    int e = 0;
    if( (proc_x != comm->myloc[0]) || (proc_y != comm->myloc[1]) || (proc_z != comm->myloc[2]) ) e = 1;
    int err = 0;
    MPI_Allreduce(&e, &err, 1, MPI_INT, MPI_SUM, world);
    if(err) error->all(FLERR,"[MBX] Inconsistent proc mapping; 'processors * * * map xyz' required for PME solver");
  }
  
  // setup json, if requested

  if(use_json) {
    
    int size = 0;
    if(me == 0) {

      // Test if file present
      FILE * fp = fopen(json_file,"r");
      if(fp == NULL) {
	char str[128];
	snprintf(str,128,"Cannot open file %s",json_file);
	error->one(FLERR,str);
      } else fclose(fp);
      
      std::ifstream t(json_file);
      t.seekg(0, std::ios::end);
      size = t.tellg();
      json_settings.resize(size);
      t.seekg(0);
      t.read(&json_settings[0], size);
    }

    MPI_Bcast(&size, 1, MPI_INT, 0, world);
    if(me) json_settings.resize(size);

    MPI_Bcast(&json_settings[0], size+1, MPI_CHAR, 0, world);
  }

  if(screen && universe->iworld == 0 && comm->me == 0) 
    std::cout << "[" << me << "] json_settings= " << json_settings << std::endl;
  
  memory->create(mbxt_count,      MBXT_NUM_TIMERS, "fixmbx:mbxt_count");
  memory->create(mbxt_time,       MBXT_NUM_TIMERS, "fixmbx:mbxt_time");
  memory->create(mbxt_time_start, MBXT_NUM_TIMERS, "fixmbx:mbxt_time_start");

  for(int i=0; i<MBXT_NUM_TIMERS; ++i) {
    mbxt_time[i] = 0.0;
    mbxt_count[i] = 0;
  }

  first_step = true;

  // MBX currently requires 4-byte tags

  if( sizeof(tagint) != sizeof(int) )
    error->all(FLERR,"[MBX] Tagints required to be of type int.");
  
  mbxt_initial_time = MPI_Wtime();
}

/* ---------------------------------------------------------------------- */

FixMBX::~FixMBX()
{
  memory->destroy(mol_offset);
  memory->destroy(mol_names);
  memory->destroy(num_mols);
  
  // unregister callbacks to this fix from Atom class

  atom->delete_callback(id,0);
  atom->delete_callback(id,1);

  memory->destroy(mol_local);
  memory->destroy(mol_type);
  memory->destroy(mol_anchor);

  if(ptr_mbx) delete ptr_mbx;

  if(ptr_mbx_full) delete ptr_mbx_full;

  if(ptr_mbx_local) {
    
    // accumulate timing info from pme electrostatics
    
    std::vector<size_t> tmpi = ptr_mbx_local->GetInfoElectrostaticsCounts();
    std::vector<double> tmpd = ptr_mbx_local->GetInfoElectrostaticsTimings();
    
    for(int i=0; i<tmpi.size(); ++i) {
      mbxt_count[MBXT_ELE_PERMDIP_REAL+i] += tmpi[i];
      mbxt_time[MBXT_ELE_PERMDIP_REAL+i] += tmpd[i];
    }

    delete ptr_mbx_local;
  }
  
  memory->destroy(mol_anchor_full);
  memory->destroy(mol_type_full);
  memory->destroy(x_full);
  memory->destroy(f_full);
  memory->destroy(f_local);
  memory->destroy(tag_full);
  memory->destroy(atom_map_full);
  memory->destroy(nlocal_rank);
  memory->destroy(nlocal_disp);
  memory->destroy(nlocal_rank3);
  memory->destroy(nlocal_disp3);

  mbxt_write_summary(); // this and collecting times should be gated by 'timer full' request
  
  memory->destroy(mbxt_count);
  memory->destroy(mbxt_time);
  memory->destroy(mbxt_time_start);
}

/* ---------------------------------------------------------------------- */

void FixMBX::post_constructor()
{

}

/* ---------------------------------------------------------------------- */

int FixMBX::setmask()
{
  int mask = 0;
  mask |= POST_NEIGHBOR;
  mask |= MIN_POST_NEIGHBOR;
  mask |= PRE_FORCE;
  //  mask |= PRE_FORCE_RESPA;
  mask |= MIN_PRE_FORCE;
  return mask;
}

/* ---------------------------------------------------------------------- */

void FixMBX::init()
{
  if (!atom->q_flag)
    error->all(FLERR,"Fix mbx requires atom attribute q");

  ngroup = group->count(igroup);
  if (ngroup == 0) error->all(FLERR,"Fix mbx group has no atoms");

  // I don't think we need neighbor lists yet...
  
  // need a half neighbor list w/ Newton off
  // built whenever re-neighboring occurs

  // int irequest = neighbor->request(this,instance_me);
  // neighbor->requests[irequest]->pair = 0;
  // neighbor->requests[irequest]->fix = 1;
  // neighbor->requests[irequest]->newton = 2;
  // neighbor->requests[irequest]->ghost = 0;
}

/* ---------------------------------------------------------------------- */

void FixMBX::setup_post_neighbor()
{
#ifdef _DEBUG
  printf("\n[MBX] (%i,%i) Inside setup_post_neighbor()\n",universe->iworld,me);
#endif
  
  // setup after first neighbor build completes

  // allocate memory on rank 0 to hold full system data

  memory->create(mol_anchor_full, atom->natoms,    "fixmbx:mol_anchor_full");
  memory->create(mol_type_full,   atom->natoms,    "fixmbx:mol_type_full");
  memory->create(x_full,          atom->natoms, 3, "fixmbx:x_full");
  memory->create(f_full,          atom->natoms, 3, "fixmbx:f_full");
  memory->create(f_local,         atom->natoms, 3, "fixmbx:f_local"); // lazy allocation...
  memory->create(tag_full,        atom->natoms,    "fixmbx:tag_full");
  memory->create(atom_map_full,   atom->natoms+1,  "fixmbx:atom_map_full");
  memory->create(nlocal_rank,     comm->nprocs,    "fixmbx::nlocal_rank");
  memory->create(nlocal_disp,     comm->nprocs,    "fixmbx::nlocal_disp");
  memory->create(nlocal_rank3,    comm->nprocs,    "fixmbx::nlocal_rank3");
  memory->create(nlocal_disp3,    comm->nprocs,    "fixmbx::nlocal_disp3");
  
  //  printf("\n[MBX] Inside setup_post_neighbor()\n");
  
  const int nall = atom->nlocal + atom->nghost;

  post_neighbor();
  
  first_step = false;
  
#ifdef _DEBUG
  printf("[MBX] (%i,%i) Leaving setup_post_neighbor()\n",universe->iworld,me);
#endif
}

/* ---------------------------------------------------------------------- */

void FixMBX::post_neighbor()
{
  // setup after neighbor build

#ifdef _DEBUG
  printf("\n[MBX] (%i,%i) Inside post_neighbor()\n",universe->iworld,me);
#endif
  
  const int nlocal = atom->nlocal;
  const int nghost = atom->nghost;
  const int nall = nlocal + nghost;
  
  tagint * tag = atom->tag;
  int * molecule = atom->molecule;
  double ** x = atom->x;

  // do we need to pre-compute and track molecule types?

  for(int i=0; i<nall; ++i) {
    const int id = tag[i];

    for(int j=0; j<num_mol_types; ++j)
      if(id <= mol_offset[j+1]) {
	mol_type[i] = j;
	break;
      }
    
  }

  // do we need to pre-compute and track anchor-atoms?

  for(int i=0; i<nall; ++i) {
    const int mol_id = molecule[i];
    const int mtype = mol_type[i];

    if( (tag[i] - 1 - mol_offset[mtype]) % num_atoms_per_mol[mtype] == 0) mol_anchor[i] = 1;
    else mol_anchor[i] = 0;
  }
  
  // tear down existing MBX objects

  if(ptr_mbx)       delete ptr_mbx;
  if(ptr_mbx_full)  delete ptr_mbx_full;
  if(ptr_mbx_local) {

    // accumulate timing info from pme electrostatics
    
    std::vector<size_t> tmpi = ptr_mbx_local->GetInfoElectrostaticsCounts();
    std::vector<double> tmpd = ptr_mbx_local->GetInfoElectrostaticsTimings();
    
    for(int i=0; i<tmpi.size(); ++i) {
      mbxt_count[MBXT_ELE_PERMDIP_REAL+i] += tmpi[i];
      mbxt_time[MBXT_ELE_PERMDIP_REAL+i] += tmpd[i];
    }
    
    delete ptr_mbx_local;
  }

  // create main instance of MBX object
  
  ptr_mbx = new bblock::System();
  
  // check if MBX compiled with MPI

  int err = ptr_mbx->TestMPI();
  if(err == -2) mbx_mpi_enabled = false;

  // create helper MBX instances
  
  if(mbx_mpi_enabled) ptr_mbx_local = new bblock::System();
  else ptr_mbx_full  = new bblock::System();

  // initialize all MBX instances
  
  mbx_init();
  if(mbx_mpi_enabled) mbx_init_local();
  else mbx_init_full();
  
#ifdef _DEBUG
  printf("[MBX] (%i,%i) Leaving post_neighbor()\n",universe->iworld,me);
#endif
}

/* ---------------------------------------------------------------------- */

void FixMBX::setup_pre_force(int vflag)
{
  pre_force(vflag);
}

/* ---------------------------------------------------------------------- */

void FixMBX::min_setup_pre_force(int vflag)
{
  setup_pre_force(vflag);
}

/* ---------------------------------------------------------------------- */

void FixMBX::init_storage()
{

}

/* ---------------------------------------------------------------------- */

void FixMBX::pre_force(int /*vflag*/)
{
  // update coordinates in MBX objects
  
  mbx_update_xyz();
  if(mbx_mpi_enabled) mbx_update_xyz_local();
  else mbx_update_xyz_full();
}

/* ---------------------------------------------------------------------- */

void FixMBX::min_pre_force(int vflag)
{
  pre_force(vflag);
}

/* ---------------------------------------------------------------------- */

int FixMBX::pack_forward_comm(int n, int *list, double *buf,
                                  int /*pbc_flag*/, int * /*pbc*/)
{
  int m;

  // if (pack_flag == 1)
  //   for(m = 0; m < n; m++) buf[m] = d[list[m]];
  // else if (pack_flag == 2)
  //   for(m = 0; m < n; m++) buf[m] = s[list[m]];
  // else if (pack_flag == 3)
  //   for(m = 0; m < n; m++) buf[m] = t[list[m]];
  // else if (pack_flag == 4)
  //   for(m = 0; m < n; m++) buf[m] = atom->q[list[m]];
  // else if (pack_flag == 5) {
  //   m = 0;
  //   for(int i = 0; i < n; i++) {
  //     int j = 2 * list[i];
  //     buf[m++] = d[j  ];
  //     buf[m++] = d[j+1];
  //   }
  //   return m;
  // }
  return n;
}

/* ---------------------------------------------------------------------- */

void FixMBX::unpack_forward_comm(int n, int first, double *buf)
{
  int i, m;

  // if (pack_flag == 1)
  //   for(m = 0, i = first; m < n; m++, i++) d[i] = buf[m];
  // else if (pack_flag == 2)
  //   for(m = 0, i = first; m < n; m++, i++) s[i] = buf[m];
  // else if (pack_flag == 3)
  //   for(m = 0, i = first; m < n; m++, i++) t[i] = buf[m];
  // else if (pack_flag == 4)
  //   for(m = 0, i = first; m < n; m++, i++) atom->q[i] = buf[m];
  // else if (pack_flag == 5) {
  //   int last = first + n;
  //   m = 0;
  //   for(i = first; i < last; i++) {
  //     int j = 2 * i;
  //     d[j  ] = buf[m++];
  //     d[j+1] = buf[m++];
  //   }
  // }
}

/* ---------------------------------------------------------------------- */

int FixMBX::pack_reverse_comm(int n, int first, double *buf)
{
  int i, m;
  //  if (pack_flag == 5) {
    // m = 0;
    // int last = first + n;
    // for(i = first; i < last; i++) {
    //   int indxI = 2 * i;
    //   buf[m++] = q[indxI  ];
    //   buf[m++] = q[indxI+1];
    // }
    // return m;
  //  } else {
    //    for (m = 0, i = first; m < n; m++, i++) buf[m] = q[i];
    return n;
    //  }
}

/* ---------------------------------------------------------------------- */

void FixMBX::unpack_reverse_comm(int n, int *list, double *buf)
{
  // if (pack_flag == 5) {
  //   int m = 0;
  //   for(int i = 0; i < n; i++) {
  //     int indxI = 2 * list[i];
  //     q[indxI  ] += buf[m++];
  //     q[indxI+1] += buf[m++];
  //   }
  // } else {
  //   for (int m = 0; m < n; m++) q[list[m]] += buf[m];
  // }
}

/* ----------------------------------------------------------------------
   memory usage of local atom-based arrays
------------------------------------------------------------------------- */

double FixMBX::memory_usage()
{
  double bytes;

  bytes = 0;

  return bytes;
}

/* ----------------------------------------------------------------------
   allocate fictitious charge arrays
------------------------------------------------------------------------- */

void FixMBX::grow_arrays(int nmax)
{
  memory->grow(mol_type,   nmax, "fixmbx:mol_type");
  memory->grow(mol_anchor, nmax, "fixmbx:mol_anchor");
  memory->grow(mol_local,  nmax, "fixmbx:mol_local");
}

/* ----------------------------------------------------------------------
   copy values within fictitious charge arrays
------------------------------------------------------------------------- */

void FixMBX::copy_arrays(int i, int j, int /*delflag*/)
{
  mol_type[j]   = mol_type[i];
  mol_anchor[j] = mol_anchor[i];
  mol_local[j]  = mol_local[i];
}

/* ----------------------------------------------------------------------
   pack values in local atom-based array for exchange with another proc
------------------------------------------------------------------------- */

int FixMBX::pack_exchange(int i, double *buf)
{
  int n = 0;
  buf[n++] = mol_type[i];
  buf[n++] = mol_anchor[i];
  buf[n++] = mol_local[i];
  return n;
}

/* ----------------------------------------------------------------------
   unpack values in local atom-based array from exchange with another proc
------------------------------------------------------------------------- */

int FixMBX::unpack_exchange(int nlocal, double *buf)
{
  int n = 0;
  mol_type[nlocal]   = buf[n++];
  mol_anchor[nlocal] = buf[n++];
  mol_local[nlocal]  = buf[n++];
  return n;
}

/* ----------------------------------------------------------------------
   Initialize new MBX instance with all molecules
------------------------------------------------------------------------- */

void FixMBX::mbx_init()
{
  mbxt_start(MBXT_INIT);

#ifdef _DEBUG
  printf("[MBX] (%i,%i) Inside mbx_init()\n",universe->iworld,me);
#endif
  
  const int nlocal = atom->nlocal;
  const int nall = nlocal + atom->nghost;
  tagint * tag = atom->tag;
  double ** x = atom->x;

  mbx_num_atoms = 0;
  
  std::vector<size_t> molec;

  double ximage[3];
  
  // loop over all atoms on proc (local + ghost)

  int nm = 0;

  const double xlo = domain->boxlo[0];
  const double ylo = domain->boxlo[1];
  const double zlo = domain->boxlo[2];
  
  for(int i=0; i<nall; ++i) {
    
    // if anchor-atom, then add to MBX (currently assume molecule is intact)
    
    if(mol_anchor[i]) {

      std::vector<std::string> names;
      std::vector<double> xyz;

      const int mtype = mol_type[i];

      int is_local = (i<nlocal);

      int na;
      if(strcmp("h2o",mol_names[mtype])      == 0) na = 3;
      else if(strcmp("na",mol_names[mtype])  == 0) na = 1;
      else if(strcmp("cl",mol_names[mtype])  == 0) na = 1;
      else if(strcmp("he",mol_names[mtype])  == 0) na = 1;
      else if(strcmp("co2",mol_names[mtype]) == 0) na = 3;
      else if(strcmp("ch4",mol_names[mtype]) == 0) na = 5;
      else error->one(FLERR,"Unsupported molecule type in MBX"); // should never get this far...

      // ids of particles in molecule on proc
      
      tagint anchor = tag[i];

      int amap[5];
      bool add_monomer = true;
      for(int j=1; j<na; ++j) {
	amap[j] = atom->map(anchor+j);
	if(amap[j] == -1) add_monomer = false;
      }
      
      // add info
      
      if(add_monomer) {

	// add coordinates
	
	xyz.push_back(x[i][0] - xlo);
	xyz.push_back(x[i][1] - ylo);
	xyz.push_back(x[i][2] - zlo);
	
	for(int j=1; j<na; ++j) {
	  domain->closest_image(x[i], x[amap[j]], ximage);
	  xyz.push_back(ximage[0] - xlo);
	  xyz.push_back(ximage[1] - ylo);
	  xyz.push_back(ximage[2] - zlo);
	}

	// add types
	
	if(strcmp("h2o",mol_names[mtype])      == 0) {
	  names.push_back("O");
	  names.push_back("H");
	  names.push_back("H");
	}
	else if(strcmp("na",mol_names[mtype])  == 0) {
	  names.push_back("Na");
	}
	else if(strcmp("cl",mol_names[mtype])  == 0) {
	  names.push_back("Cl");
	}
	else if(strcmp("he",mol_names[mtype])  == 0) {
	  names.push_back("He");
	}
	else if(strcmp("co2",mol_names[mtype]) == 0) {
	  names.push_back("C");
	  names.push_back("O");
	  names.push_back("O");
	}
	else if(strcmp("ch4",mol_names[mtype]) == 0) {
	  names.push_back("C");
	  names.push_back("H");
	  names.push_back("H");
	  names.push_back("H");
	  names.push_back("H");
	}

	// add monomer
	
	molec.push_back(nm);
	nm++;
	
	ptr_mbx->AddMonomer(xyz, names, mol_names[mtype], is_local, anchor);
	ptr_mbx->AddMolecule(molec);

	mbx_num_atoms += na;
      }

    } // if(mol_anchor)
    
  } // for(i<nall)
  
  if(mbx_num_atoms == 0) {
    mbxt_stop(MBXT_INIT);
#ifdef _DEBUG
    printf("[MBX] (%i,%i) Leaving mbx_init()\n",universe->iworld,me);
#endif
    return;
  }
  
  int * pg = comm->procgrid;
  ptr_mbx->SetMPI(world, pg[0], pg[1], pg[2]);
  
  ptr_mbx->Initialize();
  
  // setup MBX solver(s); need to keep pbc turned off, which currently disables electrostatic solver

  std::vector<double> box;

  // set MBX solvers
  
  ptr_mbx->SetDipoleMethod("cg");
  ptr_mbx->Set2bCutoff(pair_mbx->cut_global);

  if(!domain->nonperiodic) {
    ptr_mbx->SetEwaldElectrostatics(0.6, 2.5, 6);
    ptr_mbx->SetEwaldDispersion(0.5, 2.5, 6);
  }
  
  if(use_json) ptr_mbx->SetUpFromJson(json_settings);
  
  ptr_mbx->SetPBC(box);
    
  std::vector<int> egrid = ptr_mbx->GetFFTDimensionElectrostatics(0);
  std::vector<int> dgrid = ptr_mbx->GetFFTDimensionDispersion(0);
  
  if(print_settings && first_step) {
    std::string mbx_settings_ = ptr_mbx->GetCurrentSystemConfig();
    if(screen) {
      fprintf(screen, "\n[MBX] Settings\n%s\n", mbx_settings_.c_str());
      fprintf(screen, "[MBX] electrostatics FFT grid= %i %i %i\n",egrid[0],egrid[1],egrid[2]);
      fprintf(screen, "[MBX] dispersion FFT grid= %i %i %i\n",dgrid[0],dgrid[1],dgrid[2]);
    }
    if(logfile) {
      fprintf(logfile, "\n[MBX] Settings\n%s\n", mbx_settings_.c_str());
      fprintf(logfile, "[MBX] electrostatics FFT grid= %i %i %i\n",egrid[0],egrid[1],egrid[2]);
      fprintf(logfile, "[MBX] dispersion FFT grid= %i %i %i\n",dgrid[0],dgrid[1],dgrid[2]);
    }
  }
  
#ifdef _DEBUG
  printf("[MBX] (%i,%i) Leaving mbx_init()\n",universe->iworld,me);
#endif
  
  mbxt_stop(MBXT_INIT);
}

/* ----------------------------------------------------------------------
   Initialize new MBX instance with all molecules that have local atom
------------------------------------------------------------------------- */

void FixMBX::mbx_init_local()
{ 
  mbxt_start(MBXT_INIT_LOCAL);

#ifdef _DEBUG
  printf("[MBX] (%i,%i) Inside mbx_init_local()\n",universe->iworld,me);
#endif
    
  const int nlocal = atom->nlocal;
  const int nall = nlocal + atom->nghost;
  tagint * tag = atom->tag;
  double ** x = atom->x;

  mbx_num_atoms_local = 0;
  
  for(int i=0; i<nall; ++i) mol_local[i] = 0;

  // add all local+ghost monomers
  
  for(int i=0; i<nall; ++i) {
    if(mol_anchor[i]) mol_local[i] = 1;
  }
  
  // remove ghost monomers that are periodic images of local monomer
  // -- just an artifact for small systems and PBC
  // -- should be able to remove this
  
  for(int i=nlocal; i<nall; ++i) {
    if(mol_anchor[i]) {
      const int indx = atom->map( atom->tag[i] );
      if(indx < nlocal && mol_local[indx]) mol_local[i] = 0;
    }
  }

  // remove ghost monomers that are periodic images of ghost monomer
  // -- just an artifact for small systems and PBC
  // -- should be able to remove this

  double ximage[3];
  for(int i=nlocal; i<nall-1; ++i) {
    if(mol_anchor[i] && mol_local[i]) {

      for(int j=i+1; j<nall; ++j) {
  	if(mol_anchor[j] && mol_local[j]) {
  	  domain->closest_image(x[i], x[j], ximage);
  	  double dx = x[i][0] - ximage[0];
  	  double dy = x[i][1] - ximage[1];
  	  double dz = x[i][2] - ximage[2];
  	  double rsq = dx*dx + dy*dy + dz*dz;
  	  if(rsq < 0.00001) mol_local[j] = 0;
  	}
      }
    }
  }
  
  std::vector<size_t> molec;
  
  const double xlo = domain->boxlo[0];
  const double ylo = domain->boxlo[1];
  const double zlo = domain->boxlo[2];
  
  // loop over all atoms on proc

  int nm = 0;
  
  for(int i=0; i<nall; ++i) {
    
    // if anchor-atom, then add to MBX (currently assume molecule is intact)

    if(mol_anchor[i] && mol_local[i]) {

      std::vector<std::string> names;
      std::vector<double> xyz;

      const int mtype = mol_type[i];

      int is_local = (i<nlocal);
      
      int na;
      if(strcmp("h2o",mol_names[mtype])      == 0) na = 3;
      else if(strcmp("na",mol_names[mtype])  == 0) na = 1;
      else if(strcmp("cl",mol_names[mtype])  == 0) na = 1;
      else if(strcmp("he",mol_names[mtype])  == 0) na = 1;
      else if(strcmp("co2",mol_names[mtype]) == 0) na = 3;
      else if(strcmp("ch4",mol_names[mtype]) == 0) na = 5;
      else error->one(FLERR,"Unsupported molecule type in MBX"); // should never get this far...

      // ids of particles in molecule on proc
      
      tagint anchor = tag[i];

      int amap[5];
      bool add_monomer = true;
      for(int j=1; j<na; ++j) {
	amap[j] = atom->map(anchor+j);
	if(amap[j] == -1) add_monomer = false;
      }
      
      // add info
      
      if(add_monomer) {

	// add coordinates
	
	xyz.push_back(x[i][0] - xlo);
	xyz.push_back(x[i][1] - ylo);
	xyz.push_back(x[i][2] - zlo);
	
	for(int j=1; j<na; ++j) {
	  domain->closest_image(x[i], x[amap[j]], ximage);
	  xyz.push_back(ximage[0] - xlo);
	  xyz.push_back(ximage[1] - ylo);
	  xyz.push_back(ximage[2] - zlo);
	}
	
	if(strcmp("h2o",mol_names[mtype])      == 0) {
	  names.push_back("O");
	  names.push_back("H");
	  names.push_back("H");
	}
	else if(strcmp("na",mol_names[mtype])  == 0) {
	  names.push_back("Na");
	}
	else if(strcmp("cl",mol_names[mtype])  == 0) {
	  names.push_back("Cl");
	}
	else if(strcmp("he",mol_names[mtype])  == 0) {
	  names.push_back("He");
	}
	else if(strcmp("co2",mol_names[mtype]) == 0) {
	  names.push_back("C");
	  names.push_back("O");
	  names.push_back("O");
	}
	else if(strcmp("ch4",mol_names[mtype]) == 0) {
	  names.push_back("C");
	  names.push_back("H");
	  names.push_back("H");
	  names.push_back("H");
	  names.push_back("H");
	}

	molec.push_back(nm++);
	
	ptr_mbx_local->AddMonomer(xyz, names, mol_names[mtype], is_local, anchor);
	ptr_mbx_local->AddMolecule(molec);

	mbx_num_atoms_local += na;
      }
      
    } // if(mol_anchor)
    
  } // for(i<nall)
  
  int * pg = comm->procgrid;
  ptr_mbx_local->SetMPI(world, pg[0], pg[1], pg[2]);

  int err = ptr_mbx_local->TestMPI();
  if(err == -1) error->all(FLERR, "[MBX] MPI not initialized\n");
  else if(err == -2) {
    if(me == 0 && first_step)
      error->all(FLERR,"[MBX] MPI not enabled. FULL terms computed on rank 0\n");
    mbx_mpi_enabled = false;
  }
  
  if(mbx_num_atoms_local == 0) ptr_mbx_local->InitializePME();
  else ptr_mbx_local->Initialize();
  
  // setup MBX solver(s); need to keep pbc turned off, which currently disables electrostatic solver

  std::vector<double> box;

  // set MBX solvers

  if(!domain->nonperiodic) {
    ptr_mbx_local->SetDipoleMethod("cg");
    ptr_mbx_local->Set2bCutoff(pair_mbx->cut_global);
    ptr_mbx_local->SetEwaldElectrostatics(0.6, 2.5, 6);
    ptr_mbx_local->SetEwaldDispersion(0.5, 2.5, 6);
  } else {
    ptr_mbx_local->Set2bCutoff(100.0);
  }
    
  if(use_json) ptr_mbx_local->SetUpFromJson(json_settings);
  
  ptr_mbx_local->SetPBC(box);

  if(!domain->nonperiodic) {
  
    box = std::vector<double>(9,0.0);
    
    box[0] = domain->xprd;
    
    box[3] = domain->xy;
    box[4] = domain->yprd;
    
    box[6] = domain->xz;
    box[7] = domain->yz;
    box[8] = domain->zprd;
    
  } else if(domain->xperiodic || domain->yperiodic || domain->zperiodic)
    error->all(FLERR,"System must be fully periodic or non-periodic with MBX");
    
  ptr_mbx_local->SetBoxPMElocal(box);

  ptr_mbx_local->SetPeriodicity(!domain->nonperiodic);
  
  std::vector<int> egrid = ptr_mbx_local->GetFFTDimensionElectrostatics(1);
  std::vector<int> dgrid = ptr_mbx_local->GetFFTDimensionDispersion(1);
    
  if(print_settings && first_step) {
    std::string mbx_settings_ = ptr_mbx_local->GetCurrentSystemConfig();
    if(screen) {
      fprintf(screen, "\n[MBX] 'Local' Settings\n%s\n", mbx_settings_.c_str());
      fprintf(screen, "[MBX] LOCAL electrostatics FFT grid= %i %i %i\n",egrid[0],egrid[1],egrid[2]);
      fprintf(screen, "[MBX] LOCAL dispersion FFT grid= %i %i %i\n",dgrid[0],dgrid[1],dgrid[2]);
    }
    if(logfile) {
      fprintf(logfile, "\n[MBX] 'Local' Settings\n%s\n", mbx_settings_.c_str());
      fprintf(logfile, "[MBX] LOCAL electrostatics FFT grid= %i %i %i\n",egrid[0],egrid[1],egrid[2]);
      fprintf(logfile, "[MBX] LOCAL dispersion FFT grid= %i %i %i\n",dgrid[0],dgrid[1],dgrid[2]);
    }
  }
  
#ifdef _DEBUG
  printf("[MBX] (%i,%i) Leaving mbx_init_local()\n",universe->iworld,me);
#endif
  
  mbxt_stop(MBXT_INIT_LOCAL);
}

/* ----------------------------------------------------------------------
   Initialize new MBX instance with local monomers as full system 
   (for debugging purposes)
------------------------------------------------------------------------- */

void FixMBX::mbx_init_full()
{  
  mbxt_start(MBXT_INIT_FULL);

#ifdef _DEBUG
  printf("[MBX] (%i,%i) Inside mbx_init_full()\n",universe->iworld,me);
#endif

  // gather data from other MPI ranks
  
  const int nlocal = atom->nlocal;
  const int nall = nlocal + atom->nghost;
  const int natoms = atom->natoms;
  tagint * tag = atom->tag;
  double ** x = atom->x;
  
  MPI_Gather(&nlocal, 1, MPI_INT, nlocal_rank, 1, MPI_INT, 0, world);
  
  if(comm->me == 0) {
    
    nlocal_disp[0] = 0;
    for(int i=1; i<comm->nprocs; ++i)
      nlocal_disp[i] = nlocal_disp[i-1] + nlocal_rank[i-1];

    for(int i=0; i<comm->nprocs; ++i) {
      nlocal_rank3[i] = nlocal_rank[i] * 3;
      nlocal_disp3[i] = nlocal_disp[i] * 3;
    }
    
  }

  MPI_Gatherv(mol_anchor, nlocal, MPI_INT, mol_anchor_full, nlocal_rank, nlocal_disp, MPI_INT, 0, world);
  MPI_Gatherv(mol_type,   nlocal, MPI_INT, mol_type_full,   nlocal_rank, nlocal_disp, MPI_INT, 0, world);
  MPI_Gatherv(tag,        nlocal, MPI_INT, tag_full,        nlocal_rank, nlocal_disp, MPI_INT, 0, world); // not safe if BIG
  
  MPI_Gatherv(&(x[0][0]), nlocal*3, MPI_DOUBLE, &(x_full[0][0]), nlocal_rank3, nlocal_disp3, MPI_DOUBLE, 0, world);

  // if not master rank, then nothing else to do
  
  if(comm->me) {
#ifdef _DEBUG
    printf("[MBX] (%i,%i) Leaving mbx_init_full()\n",universe->iworld,me);
#endif
    return;
  }
  
  // construct atom map
  
  for(int i=0; i<natoms; ++i) atom_map_full[ tag_full[i] ] = i;
  
  std::vector<size_t> molec;

  // loop over all atoms on proc (local + ghost)

  const double xlo = domain->boxlo[0];
  const double ylo = domain->boxlo[1];
  const double zlo = domain->boxlo[2];
  
  int nm = 0;

  mbx_num_atoms_full = 0;
  
  for(int i=0; i<natoms; ++i) {
    
    // if anchor-atom, then add to MBX (currently assume molecule is intact)
    
    if(mol_anchor_full[i]) {

      std::vector<std::string> names;
      std::vector<double> xyz;

      const int mtype = mol_type_full[i];

      int is_local = 1;

#if 1
      int na;
      if(strcmp("h2o",mol_names[mtype])      == 0) na = 3;
      else if(strcmp("na",mol_names[mtype])  == 0) na = 1;
      else if(strcmp("cl",mol_names[mtype])  == 0) na = 1;
      else if(strcmp("he",mol_names[mtype])  == 0) na = 1;
      else if(strcmp("co2",mol_names[mtype]) == 0) na = 3;
      else if(strcmp("ch4",mol_names[mtype]) == 0) na = 5;
      else error->one(FLERR,"Unsupported molecule type in MBX"); // should never get this far...

      // ids of particles in molecule on proc
      
      tagint anchor = tag_full[i];

      int amap[5];
      bool add_monomer = true;
      for(int j=1; j<na; ++j) {
	amap[j] = atom_map_full[anchor+j];
	if(amap[j] == -1) {
	  error->one(FLERR,"Molecule not intact");
	  add_monomer = false;
	}
      }

      // add info

      if(add_monomer) {

	// add coordinates
	
	xyz.push_back(x_full[i][0] - xlo);
	xyz.push_back(x_full[i][1] - ylo);
	xyz.push_back(x_full[i][2] - zlo);

	for(int j=1; j<na; ++j) {
	  xyz.push_back(x_full[ amap[j] ][0] - xlo);
	  xyz.push_back(x_full[ amap[j] ][1] - ylo);
	  xyz.push_back(x_full[ amap[j] ][2] - zlo);
	}

	// add types
	
	if(strcmp("h2o",mol_names[mtype])      == 0) {
	  names.push_back("O");
	  names.push_back("H");
	  names.push_back("H");
	}
	else if(strcmp("na",mol_names[mtype])  == 0) {
	  names.push_back("Na");
	}
	else if(strcmp("cl",mol_names[mtype])  == 0) {
	  names.push_back("Cl");
	}
	else if(strcmp("he",mol_names[mtype])  == 0) {
	  names.push_back("He");
	}
	else if(strcmp("co2",mol_names[mtype]) == 0) {
	  names.push_back("C");
	  names.push_back("O");
	  names.push_back("O");
	}
	else if(strcmp("ch4",mol_names[mtype]) == 0) {
	  names.push_back("C");
	  names.push_back("H");
	  names.push_back("H");
	  names.push_back("H");
	  names.push_back("H");
	}

	// add monomer
	
	molec.push_back(nm);
	nm++;

	ptr_mbx_full->AddMonomer(xyz, names, mol_names[mtype], is_local, anchor);
	ptr_mbx_full->AddMolecule(molec);

	mbx_num_atoms_full += na;
      }
      
#else // block to be removed...
      if(strcmp("h2o",mol_names[mtype]) == 0) {

	// add water molecule
	
	tagint anchor = tag_full[i];
	names.push_back("O");
	xyz.push_back(x_full[i][0] - xlo);
	xyz.push_back(x_full[i][1] - ylo);
	xyz.push_back(x_full[i][2] - zlo);

	int ii = atom_map_full[anchor+1];
	if(ii < 0) error->one(FLERR,"H2O molecule not intact");

	names.push_back("H");
	xyz.push_back(x_full[ii][0] - xlo);
	xyz.push_back(x_full[ii][1] - ylo);
	xyz.push_back(x_full[ii][2] - zlo);

	ii = atom_map_full[anchor+2];
	if(ii < 0) error->one(FLERR,"H2O molecule not intact");
	
	names.push_back("H");
	xyz.push_back(x_full[ii][0] - xlo);
	xyz.push_back(x_full[ii][1] - ylo);
	xyz.push_back(x_full[ii][2] - zlo);

	molec.push_back(nm);
	nm++;

	ptr_mbx_full->AddMonomer(xyz, names, "h2o", is_local, anchor);
	ptr_mbx_full->AddMolecule(molec);

	mbx_num_atoms_full += 3;
	
      } else if(strcmp("na",mol_names[mtype]) == 0) {
	
	// add sodium ion

	tagint anchor = tag_full[i];
	names.push_back("Na");
	xyz.push_back(x_full[i][0] - xlo);
	xyz.push_back(x_full[i][1] - ylo);
	xyz.push_back(x_full[i][2] - zlo);

	molec.push_back(nm++);

	ptr_mbx_full->AddMonomer(xyz, names, "na", is_local, anchor);
	ptr_mbx_full->AddMolecule(molec);

	mbx_num_atoms_full++;
	
      } else if(strcmp("cl",mol_names[mtype]) == 0) {
	
	// add chloride ion

	tagint anchor = tag_full[i];
	names.push_back("Cl");
	xyz.push_back(x_full[i][0] - xlo);
	xyz.push_back(x_full[i][1] - ylo);
	xyz.push_back(x_full[i][2] - zlo);

	molec.push_back(nm++);

	ptr_mbx_full->AddMonomer(xyz, names, "cl", is_local, anchor);
	ptr_mbx_full->AddMolecule(molec);

	mbx_num_atoms_full++;

      } else if(strcmp("he",mol_names[mtype]) == 0) {

	// add helium atom
	
	tagint anchor = tag_full[i];
	names.push_back("He");
	xyz.push_back(x_full[i][0] - xlo);
	xyz.push_back(x_full[i][1] - ylo);
	xyz.push_back(x_full[i][2] - zlo);
	
	molec.push_back(nm++);
	
	ptr_mbx_full->AddMonomer(xyz, names, "he", is_local, anchor);
	ptr_mbx_full->AddMolecule(molec);
	
	mbx_num_atoms_full++;
	
      } else if(strcmp("co2",mol_names[mtype]) == 0) {
	
	// add CO2 molecule

	tagint anchor = tag_full[i];
	names.push_back("C");
	xyz.push_back(x_full[i][0] - xlo);
	xyz.push_back(x_full[i][1] - ylo);
	xyz.push_back(x_full[i][2] - zlo);

	int ii = atom_map_full[anchor+1];
	if(ii < 0) error->one(FLERR,"CO2 molecule not intact");

	names.push_back("O");
	xyz.push_back(x_full[ii][0] - xlo);
	xyz.push_back(x_full[ii][1] - ylo);
	xyz.push_back(x_full[ii][2] - zlo);

	ii = atom_map_full[anchor+2];
	if(ii < 0) error->one(FLERR,"CO2 molecule not intact");
	
	names.push_back("O");
	xyz.push_back(x_full[ii][0] - xlo);
	xyz.push_back(x_full[ii][1] - ylo);
	xyz.push_back(x_full[ii][2] - zlo);

	molec.push_back(nm++);

	ptr_mbx_full->AddMonomer(xyz, names, "co2", is_local, anchor);
	ptr_mbx_full->AddMolecule(molec);

	mbx_num_atoms_full += 3;
	  
      } else if(strcmp("ch4",mol_names[mtype]) == 0) {

	// add CH4 molecule
	
	tagint anchor = tag_full[i];
	names.push_back("C");
	xyz.push_back(x_full[i][0] - xlo);
	xyz.push_back(x_full[i][1] - ylo);
	xyz.push_back(x_full[i][2] - zlo);
	
	int ii = atom_map_full[anchor+1];
	if(ii < 0) error->one(FLERR,"CH4 molecule not intact");
	
	names.push_back("H");
	xyz.push_back(x_full[ii][0] - xlo);
	xyz.push_back(x_full[ii][1] - ylo);
	xyz.push_back(x_full[ii][2] - zlo);
	
	ii = atom_map_full[anchor+2];
	if(ii < 0) error->one(FLERR,"CH4 molecule not intact");
	
	names.push_back("H");
	xyz.push_back(x_full[ii][0] - xlo);
	xyz.push_back(x_full[ii][1] - ylo);
	xyz.push_back(x_full[ii][2] - zlo);
	
	ii = atom_map_full[anchor+3];
	if(ii < 0) error->one(FLERR,"CH4 molecule not intact");
	
	names.push_back("H");
	xyz.push_back(x_full[ii][0] - xlo);
	xyz.push_back(x_full[ii][1] - ylo);
	xyz.push_back(x_full[ii][2] - zlo);
	
	ii = atom_map_full[anchor+4];
	if(ii < 0) error->one(FLERR,"CH4 molecule not intact");
	
	names.push_back("H");
	xyz.push_back(x_full[ii][0] - xlo);
	xyz.push_back(x_full[ii][1] - ylo);
	xyz.push_back(x_full[ii][2] - zlo);
	
	molec.push_back(nm++);
	
	ptr_mbx_full->AddMonomer(xyz, names, "ch4", is_local, anchor);
	ptr_mbx_full->AddMolecule(molec);
	
	mbx_num_atoms_full += 5;

      } else error->one(FLERR,"Unsupported molecule type in MBX"); // should never get this far...
#endif	
    } // if(mol_anchor)
    
  } // for(i<nall)
  
  ptr_mbx_full->Initialize();
  
  // setup MBX solver(s)

  std::vector<double> box;

  if(!domain->nonperiodic) {
    
    box = std::vector<double>(9,0.0);
    
    box[0] = domain->xprd;
    
    box[3] = domain->xy;
    box[4] = domain->yprd;
    
    box[6] = domain->xz;
    box[7] = domain->yz;
    box[8] = domain->zprd;
    
  } else if(domain->xperiodic || domain->yperiodic || domain->zperiodic)
    error->one(FLERR,"System must be fully periodic or non-periodic with MBX");

  // set MBX solvers
  
  ptr_mbx_full->SetDipoleMethod("cg");
  if (box.size()) {
    ptr_mbx_full->Set2bCutoff(pair_mbx->cut_global);
    ptr_mbx_full->SetEwaldElectrostatics(0.6, 2.5, 6);
    ptr_mbx_full->SetEwaldDispersion(0.5, 2.5, 6);
  } else {
    ptr_mbx_full->Set2bCutoff(100.0);
  }
  
  if(use_json) ptr_mbx_full->SetUpFromJson(json_settings);

  ptr_mbx_full->SetPBC(box);
  
  std::vector<int> egrid = ptr_mbx_full->GetFFTDimensionElectrostatics(0);
  std::vector<int> dgrid = ptr_mbx_full->GetFFTDimensionDispersion(0);
  
  if(print_settings && first_step) {
    std::string mbx_settings_ = ptr_mbx_full->GetCurrentSystemConfig();
    if(screen) {
      fprintf(screen, "\n[MBX] 'Full' Settings\n%s\n", mbx_settings_.c_str()); 
      fprintf(screen, "[MBX] FULL electrostatics FFT grid= %i %i %i\n",egrid[0],egrid[1],egrid[2]);
      fprintf(screen, "[MBX] FULL dispersion FFT grid= %i %i %i\n",dgrid[0],dgrid[1],dgrid[2]);
    }
    if(logfile) {
      fprintf(logfile, "\n[MBX] 'Full' Settings\n%s\n", mbx_settings_.c_str());
      fprintf(logfile, "[MBX] FULL electrostatics FFT grid= %i %i %i\n",egrid[0],egrid[1],egrid[2]);
      fprintf(logfile, "[MBX] FULL dispersion FFT grid= %i %i %i\n",dgrid[0],dgrid[1],dgrid[2]);
    }
  }
  
#ifdef _DEBUG
  printf("[MBX] (%i,%i) Leaving mbx_init_full()\n",universe->iworld,me);
#endif
  
  mbxt_stop(MBXT_INIT_FULL);
}

/* ----------------------------------------------------------------------
   Update MBX instance for all molecules
------------------------------------------------------------------------- */

void FixMBX::mbx_update_xyz()
{
  mbxt_start(MBXT_UPDATE_XYZ);

#ifdef _DEBUG
  printf("[MBX] (%i,%i) Inside mbx_update_xyz()\n",universe->iworld,me);
#endif

  // update coordinates
  
  const int nlocal = atom->nlocal;
  const int nall = nlocal + atom->nghost;
  tagint * tag = atom->tag;
  double ** x = atom->x;

  if(mbx_num_atoms == 0) {
    mbxt_stop(MBXT_UPDATE_XYZ);
    return;
  }
  
  const double xlo = domain->boxlo[0];
  const double ylo = domain->boxlo[1];
  const double zlo = domain->boxlo[2];
  
  double ximage[3];
  
  std::vector<double> xyz(mbx_num_atoms*3);

  int indx = 0;
  for(int i=0; i<nall; ++i) {

    if(mol_anchor[i]) {

      const int mtype = mol_type[i];

      int na = 0;
      if(strcmp("h2o",mol_names[mtype])      == 0) na = 3;
      else if(strcmp("na",mol_names[mtype])  == 0) na = 1;
      else if(strcmp("cl",mol_names[mtype])  == 0) na = 1;
      else if(strcmp("he",mol_names[mtype])  == 0) na = 1;
      else if(strcmp("co2",mol_names[mtype]) == 0) na = 3;
      else if(strcmp("ch4",mol_names[mtype]) == 0) na = 5;
      else error->one(FLERR,"Unsupported molecule type in MBX"); // should never get this far...

      // ids of particles in molecule on proc
      
      tagint anchor = tag[i];

      int amap[5];
      bool add_monomer = true;
      for(int j=1; j<na; ++j) {
	amap[j] = atom->map(anchor+j);
	if(amap[j] == -1) add_monomer = false;
      }
      
      // add info
      
      if(add_monomer) {

	// add coordinates
	
	xyz[indx*3  ] = x[i][0] - xlo;
	xyz[indx*3+1] = x[i][1] - ylo;
	xyz[indx*3+2] = x[i][2] - zlo;
	
	for(int j=1; j<na; ++j) {
	  domain->closest_image(x[i], x[amap[j]], ximage);
	  xyz[(indx+j)*3  ] = ximage[0] - xlo;
	  xyz[(indx+j)*3+1] = ximage[1] - ylo;
	  xyz[(indx+j)*3+2] = ximage[2] - zlo;
	}

	indx += na;
      }

    } // if(mol_anchor)

  } // for(i<nall)

  if(xyz.size() != indx*3) error->one(FLERR,"Inconsistent # of atoms");
  ptr_mbx->SetRealXyz(xyz);

#ifdef _DEBUG
  printf("[MBX] (%i,%i) Leaving mbx_update_xyz()\n",universe->iworld,me);
#endif
  
  mbxt_stop(MBXT_UPDATE_XYZ);
}

/* ----------------------------------------------------------------------
   Update MBX instance for local molecules + plus halo
------------------------------------------------------------------------- */

void FixMBX::mbx_update_xyz_local()
{  
  mbxt_start(MBXT_UPDATE_XYZ_LOCAL);

#ifdef _DEBUG
  printf("[MBX] (%i,%i) Inside mbx_update_xyz_local()\n",universe->iworld,me);
#endif

  // update if box changes
  // need to update box passed to PME solver

  if(domain->box_change) {
    std::vector<double> box;

    if(!domain->nonperiodic) {
    
      box = std::vector<double>(9,0.0);
      
      box[0] = domain->xprd;
      
      box[3] = domain->xy;
      box[4] = domain->yprd;
      
      box[6] = domain->xz;
      box[7] = domain->yz;
      box[8] = domain->zprd;
      
    } else if(domain->xperiodic || domain->yperiodic || domain->zperiodic)
      error->all(FLERR,"System must be fully periodic or non-periodic with MBX");
  
    ptr_mbx_local->SetBoxPMElocal(box);
  }

  // update coordinates
  
  const int nlocal = atom->nlocal;
  const int nall = nlocal + atom->nghost;
  tagint * tag = atom->tag;
  double ** x = atom->x;

  if(mbx_num_atoms_local == 0) {
    mbxt_stop(MBXT_UPDATE_XYZ);
    return;
  }
  
  const double xlo = domain->boxlo[0];
  const double ylo = domain->boxlo[1];
  const double zlo = domain->boxlo[2];
  
  double ximage[3];
  
  std::vector<double> xyz(mbx_num_atoms_local*3);

  int indx = 0;
  for(int i=0; i<nall; ++i) {

    if(mol_anchor[i] && mol_local[i]) {

      const int mtype = mol_type[i];
      
      int na = 0;
      if(strcmp("h2o",mol_names[mtype])      == 0) na = 3;
      else if(strcmp("na",mol_names[mtype])  == 0) na = 1;
      else if(strcmp("cl",mol_names[mtype])  == 0) na = 1;
      else if(strcmp("he",mol_names[mtype])  == 0) na = 1;
      else if(strcmp("co2",mol_names[mtype]) == 0) na = 3;
      else if(strcmp("ch4",mol_names[mtype]) == 0) na = 5;
      else error->one(FLERR,"Unsupported molecule type in MBX"); // should never get this far...

      // ids of particles in molecule on proc
      
      tagint anchor = tag[i];

      int amap[5];
      bool add_monomer = true;
      for(int j=1; j<na; ++j) {
	amap[j] = atom->map(anchor+j);
	if(amap[j] == -1) add_monomer = false;
      }
      
      // add info
      
      if(add_monomer) {

	// add coordinates
	
	xyz[indx*3  ] = x[i][0] - xlo;
	xyz[indx*3+1] = x[i][1] - ylo;
	xyz[indx*3+2] = x[i][2] - zlo;
	
	for(int j=1; j<na; ++j) {
	  domain->closest_image(x[i], x[amap[j]], ximage);
	  xyz[(indx+j)*3  ] = ximage[0] - xlo;
	  xyz[(indx+j)*3+1] = ximage[1] - ylo;
	  xyz[(indx+j)*3+2] = ximage[2] - zlo;
	}

	indx += na;
      }

    } // if(mol_anchor)

  } // for(i<nall)
  
  if(xyz.size() != indx*3) error->one(FLERR,"Inconsistent # of atoms");
  ptr_mbx_local->SetRealXyz(xyz);

#ifdef _DEBUG
  printf("[MBX] (%i,%i) Leaving mbx_update_xyz_local()\n",universe->iworld,me);
#endif
  
  mbxt_stop(MBXT_UPDATE_XYZ_LOCAL);
}

/* ----------------------------------------------------------------------
   Update MBX instance for all molecules
------------------------------------------------------------------------- */

void FixMBX::mbx_update_xyz_full()
{  
  mbxt_start(MBXT_UPDATE_XYZ_FULL);

#ifdef _DEBUG
  printf("[MBX] (%i,%i) Inside mbx_update_xyz_full()\n",universe->iworld,me);
#endif

  // gather coordinates
  
  const int nlocal = atom->nlocal;
  const int natoms = atom->natoms;
  tagint * tag = atom->tag;
  double ** x = atom->x;

  // update coordinates

  MPI_Gatherv(&(x[0][0]), nlocal*3, MPI_DOUBLE, &(x_full[0][0]), nlocal_rank3, nlocal_disp3, MPI_DOUBLE, 0, world);  

  // if not master rank, then nothing else to do

  if(comm->me) {
    mbxt_stop(MBXT_UPDATE_XYZ_FULL);
    return;
  }
  
  // update if box changes

  if(domain->box_change) {
    
    std::vector<double> box;

    if(!domain->nonperiodic) {
    
      box = std::vector<double>(9,0.0);
      
      box[0] = domain->xprd;
      
      box[3] = domain->xy;
      box[4] = domain->yprd;
      
      box[6] = domain->xz;
      box[7] = domain->yz;
      box[8] = domain->zprd;
      
    } else if(domain->xperiodic || domain->yperiodic || domain->zperiodic)
      error->one(FLERR,"System must be fully periodic or non-periodic with MBX");
  
    ptr_mbx_full->SetPBC(box);
  }

  // update coordinates
  
  const double xlo = domain->boxlo[0];
  const double ylo = domain->boxlo[1];
  const double zlo = domain->boxlo[2];
  
  std::vector<double> xyz(natoms*3);
  
  int indx = 0;
  for(int i=0; i<natoms; ++i) {

    if(mol_anchor_full[i]) {

      const int mtype = mol_type_full[i];

      int na = 0;

      if(     strcmp("h2o", mol_names[mtype]) == 0) na = 3;
      else if(strcmp("na",  mol_names[mtype]) == 0) na = 1;
      else if(strcmp("cl",  mol_names[mtype]) == 0) na = 1;
      else if(strcmp("he",  mol_names[mtype]) == 0) na = 1;
      else if(strcmp("co2", mol_names[mtype]) == 0) na = 3;
      else if(strcmp("ch4", mol_names[mtype]) == 0) na = 5;
	
      tagint anchor = tag_full[i];
      
      xyz[indx*3  ] = x_full[i][0] - xlo;
      xyz[indx*3+1] = x_full[i][1] - ylo;
      xyz[indx*3+2] = x_full[i][2] - zlo;

      for(int j=1; j<na; ++j) {
	int ii = atom_map_full[anchor + j];
	int jndx = 3 * j;
	xyz[indx*3 + jndx  ] = x_full[ii][0] - xlo;
	xyz[indx*3 + jndx+1] = x_full[ii][1] - ylo;
	xyz[indx*3 + jndx+2] = x_full[ii][2] - zlo;
      }

      indx += na;

    } // if(mol_anchor)

  } // for(i<nall)

  if(xyz.size() != indx*3) error->one(FLERR,"Inconsistent # of atoms");
  ptr_mbx_full->SetRealXyz(xyz);

#ifdef _DEBUG
  printf("[MBX] (%i,%i) Leaving mbx_update_xyz_full()\n",universe->iworld,me);
#endif
  
  mbxt_stop(MBXT_UPDATE_XYZ_FULL);
}

/* ----------------------------------------------------------------------
   Helper functions for timing
------------------------------------------------------------------------- */

void FixMBX::mbxt_start(int T)
{
  mbxt_time_start[T] = MPI_Wtime();
}

void FixMBX::mbxt_stop(int T)
{
  mbxt_time[T] += MPI_Wtime() - mbxt_time_start[T];
  mbxt_count[T]++;
}

void FixMBX::mbxt_print_time(const char * name, int T, double * d)
{
  double tavg = d[T];
  double tmin = d[MBXT_NUM_TIMERS + T];
  double tmax = d[MBXT_NUM_TIMERS*2 + T];

  double p = tmax / d[MBXT_NUM_TIMERS*3] * 100.0;
  
  if(screen)
    fprintf(screen,"[MBX] %-20s:  %12.5g  %12.5g  %12.5g  %8i %8.2f%%\n",name,tmin,tavg,tmax,mbxt_count[T],p);

  if(logfile)
    fprintf(logfile,"[MBX] %-20s:  %12.5g  %12.5g  %12.5g  %8i %8.2f%%\n",name,tmin,tavg,tmax,mbxt_count[T],p);
}

void FixMBX::mbxt_write_summary()
{  
  double t[MBXT_NUM_TIMERS*3+1];
  double * tavg = &t[0];
  double * tmin = &t[MBXT_NUM_TIMERS];
  double * tmax = &t[MBXT_NUM_TIMERS*2];

  // total runtime since fix created
  
  t[MBXT_NUM_TIMERS*3] = MPI_Wtime() - mbxt_initial_time;

  MPI_Reduce(mbxt_time, tavg, MBXT_NUM_TIMERS, MPI_DOUBLE, MPI_SUM, 0, world);
  MPI_Reduce(mbxt_time, tmin, MBXT_NUM_TIMERS, MPI_DOUBLE, MPI_MIN, 0, world);
  MPI_Reduce(mbxt_time, tmax, MBXT_NUM_TIMERS, MPI_DOUBLE, MPI_MAX, 0, world);

  if(me) return;
  
  for(int i=0; i<MBXT_NUM_TIMERS; ++i) tavg[i] /= (double)nprocs;

  if(screen) {
    fprintf(screen,"\n[MBX] Total MBX fix/pair time= %f seconds\n",t[MBXT_NUM_TIMERS*3]);
    fprintf(screen,"[MBX] Timing Summary\n");
    fprintf(screen,"[MBX] kernel                      tmin          tavg          tmax         count   %%total\n");
    fprintf(screen,"[MBX] -----------------------------------------------------------------------------------\n");
  }
  if(logfile) {
    fprintf(logfile,"\n[MBX] Total MBX fix/pair time= %f seconds\n",t[MBXT_NUM_TIMERS*3]);
    fprintf(logfile,"[MBX] Timing Summary\n");
    fprintf(logfile,"[MBX] kernel                      tmin          tavg          tmax         count   %%total\n");
    fprintf(logfile,"[MBX] -----------------------------------------------------------------------------------\n");
  }
  
  mbxt_print_time("INIT",            MBXT_INIT,         t);
  mbxt_print_time("UPDATE_XYZ",      MBXT_UPDATE_XYZ,   t);
  mbxt_print_time("ACCUMULATE_F",    MBXT_ACCUMULATE_F, t);
  
  mbxt_print_time("E1B",       MBXT_E1B,       t);
  mbxt_print_time("E2B_LOCAL", MBXT_E2B_LOCAL, t);
  mbxt_print_time("E2B_GHOST", MBXT_E2B_GHOST, t);
  mbxt_print_time("E3B_LOCAL", MBXT_E3B_LOCAL, t);
  mbxt_print_time("E3B_GHOST", MBXT_E3B_GHOST, t);
  mbxt_print_time("DISP",      MBXT_DISP,      t);
  mbxt_print_time("DISP_PME",  MBXT_DISP_PME,  t);
  mbxt_print_time("BUCK",      MBXT_BUCK,      t);
  mbxt_print_time("ELE",       MBXT_ELE,       t);
  
  mbxt_print_time("INIT_FULL",         MBXT_INIT_FULL,         t);
  mbxt_print_time("UPDATE_XYZ_FULL",   MBXT_UPDATE_XYZ_FULL,   t);
  mbxt_print_time("ACCUMULATE_F_FULL", MBXT_ACCUMULATE_F_FULL, t);
  
  mbxt_print_time("INIT_LOCAL",         MBXT_INIT_LOCAL,         t);
  mbxt_print_time("UPDATE_XYZ_LOCAL",   MBXT_UPDATE_XYZ_LOCAL,   t);
  mbxt_print_time("ACCUMULATE_F_LOCAL", MBXT_ACCUMULATE_F_LOCAL, t);

  if(screen) {
    fprintf(screen,"\n\n[MBX] Electrostatics Summary\n");
    fprintf(screen,"[MBX] kernel                      tmin          tavg          tmax         count   %%total\n");
    fprintf(screen,"[MBX] -----------------------------------------------------------------------------------\n");
  }
  if(logfile) {
    fprintf(logfile,"\n\n[MBX] Electrostatics Summary\n");
    fprintf(logfile,"[MBX] kernel                      tmin          tavg          tmax         count   %%total\n");
    fprintf(logfile,"[MBX] -----------------------------------------------------------------------------------\n");
  }

  mbxt_print_time("ELE_PERMDIP_REAL", MBXT_ELE_PERMDIP_REAL, t);
  mbxt_print_time("ELE_PERMDIP_PME",  MBXT_ELE_PERMDIP_PME,  t);
  
  mbxt_print_time("ELE_DIPFIELD_REAL", MBXT_ELE_DIPFIELD_REAL, t);
  mbxt_print_time("ELE_DIPFIELD_PME",  MBXT_ELE_DIPFIELD_PME,  t);
  
  mbxt_print_time("ELE_GRAD_REAL",   MBXT_ELE_GRAD_REAL,   t);
  mbxt_print_time("ELE_GRAD_PME",    MBXT_ELE_GRAD_PME,    t);
  mbxt_print_time("ELE_GRAD_FIN",    MBXT_ELE_GRAD_FIN,    t);
  mbxt_print_time("ELE_COMM_REVFOR", MBXT_ELE_COMM_REVFOR, t);
}
