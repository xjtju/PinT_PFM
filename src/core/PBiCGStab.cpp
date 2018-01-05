// PBiCBSTAB 
#include "PBiCGStab.h"

void PBiCGStab::solve(){

    double *x = fetch();  // get the grid variables

    double rho0, rho, alpha, omega, beta;
    double res = 1000.0;
   
    rho0 = alpha = omega = 1.0;     
      
    clear_mem(v, size);
    clear_mem(b, size);
    clear_mem(r, size);
    clear_mem(p, size);
    clear_mem(p_, size);

    grid->guardcell(x);    
    cg_b(x);

    cg_rk(r, x, b); //init residual r = b - Ax 
    //grid->output_var(r,false);
    // choose an aribtrary vector r0_ such that (r0_, r) /=0, e.g. r0_ = r
    blas_cp(r0_, r, size);

    double tmp, tmp1, tmp2; 
    tmp = tmp1 = tmp2 = 0.0;  

    double b_nrm2 = cg_vdot(b, b);
    grid->sp_allreduce(&b_nrm2);
    int i;
    for(i=1; i<=itmax; i++){
        rho = cg_vdot(r0_, r);
        grid->sp_allreduce(&rho);
        //if( i==1 ){
        //   blas_cp(p, r, size);
        //}else {
            beta = (rho/rho0) * (alpha/omega);
            cg_direct(p, r, v, beta, omega); 
       //}

        grid->guardcell(p);
        preconditioner(p_,p); //solve Mp_=p, in some algorithm description, p_ is also denoted by y

        grid->guardcell(p_);
        cg_Xv(v,p_);        // v=Ap_    

        tmp = cg_vdot(r0_, v);
        grid->sp_allreduce(&tmp);
        alpha = rho / tmp; 
        // h = x + alpha*p_, is ||h|| is sufficiently small, can do...  
        
        cg_avpy(s, alpha, r, v);  // s = r -alpha*v;
        grid->guardcell(s);
        // solve Ms_=s , in some algorithm description, s_ is also denoted by z
        preconditioner(s_,s); 
        
        grid->guardcell(s_);
        cg_Xv(t,s_); // t=Az

        // in preconditioner t = 1/M_1*t and s = 1/M_1*s
        tmp1 = cg_vdot(t, s);
        grid->sp_allreduce(&tmp1);
        tmp2 = cg_vdot(t, t); // t dot t 
        grid->sp_allreduce(&tmp2);
        omega = tmp1 / tmp2;   
        
        //printf("%d rho : %f, beta : %f, alpha : %f, omega : %f \n",i,  rho, beta, alpha, omega);

        cg_xi(x, alpha, p_, omega, s_); // xi = xi_1 + alpha*y + omega*z;   
        
        cg_avpy(r, omega, s, t);  // r = s - omega*t
         
        // ||r|| 
        tmp = cg_vdot(r, r);
        grid->sp_allreduce(&tmp);   
        res = sqrt(tmp/b_nrm2);

        if(res < eps) {
            break;
        }
        rho0 = rho;
        grid->bc(x);
    }

    grid->guardcell(x);
    update(); // update grid variables
}

//
// vector operations can be accelerated by SIMD. 
//

//p = r + beta * ( p - omg * v )
void PBiCGStab::cg_direct1d(double* p, double* r, double* v, double beta, double omega){
    for(int i=nguard; i<nx+nguard; i++) {
        p[i] = r[i] + beta*(p[i] - omega*v[i]);
    }
}

// x = x + ay + bz  
void PBiCGStab::cg_xi1d(double* x, double* y, double* z, double alpha, double omega){
    for(int i=nguard; i<nx+nguard; i++) {
        x[i] = x[i] + alpha*y[i] + omega*z[i];
    }
}

//
// 2D, 
// NOTE: the following 2d functions are for test only, in practice, their fortran counterparts are used instead 
//

void PBiCGStab::cg_direct2d(double* p, double* r, double* v, double beta, double omega){
    long idx;
    for(int j=nguard; j<ny+nguard; j++)
        for(int i=nguard; i<nx+nguard; i++) {
            idx = grid->getOuterIdx(i,j);
            p[idx] = r[idx] + beta*(p[idx] - omega*v[idx]);
    }
}

void PBiCGStab::cg_xi2d(double* x, double* y, double* z, double alpha, double omega){
    long idx;
    for(int j=nguard; j<ny+nguard; j++) 
        for(int i=nguard; i<nx+nguard; i++) {
            idx = grid->getOuterIdx(i,j); 
            x[idx] = x[idx] + alpha*y[idx] + omega*z[idx];
    }
}

/**
  preconditioner, in current version , there is no precondition
  solve Mp_=p
  M = M1*M2 ~ A, there is a simple method for decomposing the matrix A when A has some regular pattern, 
  for example in HEAT diffusion:
  if we set  M1=I, M2=A, where I is Identity matrix. 
  than the preconditioner can be set to a light-weight but less precise solver than BiCG, and solving the same linear system: Ax=b, 
  moreover the following calculations of BiCG will become : 
    ==  reverse(M1)*t=I*t=t and reverse(M1)*s=I*s=s ==  
  so the matrix reverse and matrix multiplying vector calculations can also be skipped.      
*/
void PBiCGStab::preconditioner(double* p_, double* p){
    blas_cp(p_, p, size);
}

void PBiCGStab::update() {
    //in current implementation, the solver directly modify the grid variables, 
    //so it is not necessary to do any real operations for updating 
}
