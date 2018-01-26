#ifndef _PinT_PFMSOLVER_H_
#define _PinT_PFMSOLVER_H_ 1

#include "PBiCGStab.h"
#include "pfm.h"

/**
 * Phase Field Model using Newton-Raphson 
 *
 * Allen-Cahn Equation (AC)
 *
 * Unlike the classic Heat equation with constant diffuse coefficient, 
 * the discrete formula of the AC equation based on Crank-Nicolson is nonlinear, 
 * so Newton-Raphson method is used to linearize the numerical formula of AC, and the linear solver can be applied to AC.
 * Compared to the HEAT, it is introduced extra three variables for data structure preservation during the calculation 
 * due to its nonlinear feature.
 *
 * NOTE: 
 *  The class is a template of Newton-Raphson method, the default finite difference method is Crank-Nicolson (when theta=0.5), 
 *  other differentiation formulas can be easily implemented by overwriting all the virtual functions.
 *  See the BD4Solver (4th order backward euler) for example.
 *
 * If using the default configuration, k=16000, d=1, beta=-0.128, 
 * only after 0.1 second, the system has already reached its steady state.
 * 
 */

class PFMSolver : public Solver {

protected:
    void setup();

    virtual LS* getLS(PinT *conf, Grid *grid){
        return new PBiCGStab(conf, grid);
    }

public:

    double k;     // interfacial width related
    double d;     // diffuse coefficient 
    double beta;  // potential engrgy parameter
    double beta_;       // 0.5-beta

    double theta = 0.5;  //Crank-Nicolson; 0: Ex.Euler; 1: Im.Euler
    int newton_itmax = 5;

    double dtk;         // dt*k
    double lamda_x;     // d*dt/(dx**2)
    double lamda_y = 0.0;     
    double lamda_z = 0.0;    
    double lamdaxyz[3]; 

    double *soln;   // the current solution, pointer to the grid->u_f/u_c
    // structure preservation 
    double *soln_1;  // the holder of -F^{k-1} in Newton's method when applying to nonlinear systems of equations 
    double *G1;     // the partial RHS of Newton's method for PFM  
    double *unk;    // the unknown 'x' of Ax=b, that is (Xn+1 - Xn) for Newton's method 

    LS *hypre;  // linear solver 

    PFMSolver(PinT *c, Grid *g); 
    PFMSolver(PinT *c, Grid *g, bool isFS); 

    virtual ~PFMSolver() {

        delete hypre;

        free_mem(soln_1);
        free_mem(G1);
        free_mem(unk);

        if(grid->myid==0 && conf->verbose)
        printf("INFO: the memory allocate by PFMSolver has been released.\n");
    }

    void evolve();         // evolve over a time slice 
    void newton_raphson(); // the template of New-Raphson method iteration 
    
    /*
     * NOTE : for space division, converge check must be performed in the whole geographical space 
     */
    void chk_eps(double *err); 

    void init();
    void init1d();
    void init2d();
    void init3d();
    
    // init previous solution holder for backward method, the structure preservation
    // at the starting, all of them are identical to the initial condition of the problem 
    virtual void init_holder() {
            blas_cp_(soln_1, soln, &size); 
    }

    // update backward solution holders for the next step 
    virtual void update_holder(){
            blas_cp_(soln_1, soln, &size); 
    }
    
    virtual void stencil() {
        if(ndim==3)      stencil_ac_3d_(grid->nxyz, lamdaxyz, &nguard, bcp, soln, &theta, &dtk, &beta_);
        else if(ndim==2) stencil_ac_2d_(grid->nxyz, lamdaxyz, &nguard, bcp, soln, &theta, &dtk, &beta_);
        else if(ndim==1) stencil_ac_1d_(grid->nxyz, lamdaxyz, &nguard, bcp, soln, &theta, &dtk, &beta_);
    }

    virtual void rhs() {
        if(ndim==3)      rhs_ac_3d_(grid->nxyz, lamdaxyz, &nguard, b, soln, soln_1, G1, &theta, &dtk, &beta_);
        else if(ndim==2) rhs_ac_2d_(grid->nxyz, lamdaxyz, &nguard, b, soln, soln_1, G1, &theta, &dtk, &beta_);
        else if(ndim==1) rhs_ac_1d_(grid->nxyz, lamdaxyz, &nguard, b, soln, soln_1, G1, &theta, &dtk, &beta_);
    }
    virtual void rhs_g1() {
        if(ndim==3)      rhs_g1_ac_3d_(grid->nxyz, lamdaxyz, &nguard, soln_1,  G1, &theta, &dtk, &beta_);
        else if(ndim==2) rhs_g1_ac_2d_(grid->nxyz, lamdaxyz, &nguard, soln_1,  G1, &theta, &dtk, &beta_);
        else if(ndim==1) rhs_g1_ac_1d_(grid->nxyz, lamdaxyz, &nguard, soln_1,  G1, &theta, &dtk, &beta_);
    }

    void update() {
        if(ndim==3)      update_ac_3d_(grid->nxyz, &nguard, soln, unk);
        else if(ndim==2) update_ac_2d_(grid->nxyz, &nguard, soln, unk);
        else if(ndim==1) update_ac_1d_(grid->nxyz, &nguard, soln, unk);
    }
};

//problem specific parameter parsing
int pfm_inih(void* obj, const char* section, const char* name, const char* value);

#endif
