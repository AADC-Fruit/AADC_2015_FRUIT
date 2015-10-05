#include "rationalsupershape2d.h"

#include <iostream>
#include <cmath>

RationalSuperShape2D::RationalSuperShape2D() {
    Init(1, 1,    //scale
         2, 2, 2,  //shape
         4, 1,     //symmetric
         0,0,      //rotational
         0,0,0);   //translational
}

RationalSuperShape2D::RationalSuperShape2D(double a, double b, double n1,
                                           double n2,double n3,double p,
                                           double q, double thtOffset, double phiOffset,
                                           double xOffset, double yOffset, double zOffset) {
    Init(a, b,
         n1, n2, n3,
         p, q,
         thtOffset, phiOffset,
         xOffset, yOffset, zOffset);
}

RationalSuperShape2D::~RationalSuperShape2D() {
    mParameters.clear();
}

void RationalSuperShape2D::Init(double a, double b, double n1,
                                double n2,double n3,double p,
                                double q, double thtOffset, double phiOffset,
                                double xOffset, double yOffset, double zOffset) {
    mParameters.clear();

    mParameters.push_back(a);
    mParameters.push_back(b);

    mParameters.push_back(n1);
    mParameters.push_back(n2);
    mParameters.push_back(n3);

    mParameters.push_back(p);
    mParameters.push_back(q);

    mParameters.push_back(thtOffset);
    mParameters.push_back(phiOffset);

    mParameters.push_back(xOffset);
    mParameters.push_back(yOffset);
    mParameters.push_back(zOffset);
}


void RationalSuperShape2D::Init( double a, double b, double n1,double n2,double n3) {
    Init(a, b,              //scale
         n1, n2, n3,         //shape
         Get_p(), Get_q(),   //symmetric
         Get_thtoffset(),0,  //rotational
         0,0,0);             //translational
}

double RationalSuperShape2D::ImplicitFunction1( const Vector2d &point, vector <double> &dfFinal ) {
    dfFinal.clear();

    // nothing computable, return zero values, zero partial derivatives
    // the point will have no effect on the ongoing computations
    if (point[0] == 0 && point[1] == 0)
    {
        // dF/Dx, dF/Dy, dF/Dr set to zero...
        for (int i = 0; i < 3; i++) {
            dfFinal.push_back(0);
        }
        return 0;
    }

    vector <double> f, dDum;
    double x(point[0]), y(point[1]), psl(point.squaredNorm()), pl(sqrt(psl)), dThtDx (-y/psl), dThtDy (x/psl), r, drDtht;

    vector< vector<double> > dF;

    // assert angular values between [0, 2q*Pi]
    double tht(atan2(y,x)), thtBase(tht);
    if (tht < 0)
        thtBase += 2.0 * RATIONAL_PI;

    // compute all intersections and associated partial derivatives
    for (int i = 0; i < Get_q(); i++)
    {
        tht = thtBase + (i * 2.0 * RATIONAL_PI);

        r = Radius(tht);
        //store function
        f.push_back(r - pl);

        // store partial derivatives
        vector <double> row;
        drDtht = DrDtheta(tht);
        // df/dx
        row.push_back(drDtht*dThtDx - cos(tht));
        // df/dy
        row.push_back(drDtht*dThtDy - sin(tht));
        // df/dr
        row.push_back(1.0);
        dF.push_back(row);
    }

    // bubble sort, not really efficient but acceptable for such small arrays
    for(int i = 0; i < Get_q()-1; i++) {
        for (int j = i+1; j < Get_q(); j++) {
            if (f[i] < f[j]) {
                // swap values of f[i] and f[j]
                swap(f[i],f[j]);
                // swap rows dF[i] and dF[j]
                dF[i].swap(dF[j]);
            }
        }
    }

    // Compute resulting Rfunction
    // vector for df/dxi
    vector<double> dF1;

    // iterative evaluation of:
    // -the resulting R-functions
    // -the associated partial derivatives
    double f1,fDum;
    // first value of f
    f1 = f[0];
    // first associated row with partial derivatives
    dF1 = dF[0];

    // combine functions as (...((F1 v F2) v F3 ) v F4) v ...)
    // for all intersections
    for(int i = 1; i < Get_q(); i++)
    {
        // compute R-function, sets all partial derivatives
        // fDum and dDum temporary results of the union from F1 to Fi
        RpUnion(f1, f[i], dF1, dF[i], fDum, dDum);

        // update results in f1 and dF1, and iterate
        f1 = fDum;
        dF1 = dDum;
    }

    // final partial derivatives df/dxi after R-functions
    dfFinal = dF1;

    // clear arrays
    f.clear();
    dF.clear();
    dDum.clear();
    dF1.clear();

    // return results
    return f1;
}

double RationalSuperShape2D::ImplicitFunction2( const Vector2d &point, vector <double> &dfFinal ) {
    dfFinal.clear();

    // nothing computable, return zero values, zero partial derivatives
    // the point will have no effect on the ongoing computations
    if ( point[0] == 0 && point[1] == 0)
    {
        // dF/Dx, dF/Dy, dF/Dr set to zero...
        for (int i = 0; i < 3; i++) {
            dfFinal.push_back(0);
        }
        return 0;
    }

    vector <double> f, dDum;
    double x(point[0]), y(point[1]), psl(point.squaredNorm()), pl(sqrt(psl)), dThtDx (-y/psl), dThtDy (x/psl), r,drDtht;

    vector<vector<double> > dF;

    // assert angular values between [0, 2q*Pi]
    double tht (atan2(y,x)), thtBase(tht);
    if (tht < 0) {
        thtBase += 2.0 * RATIONAL_PI;
    }

    // compute all intersections and associated gradient values
    for (int i=0; i < Get_q(); i++)
    {
        tht = thtBase + i * 2.0 * RATIONAL_PI;
        r = Radius(tht);
        drDtht = DrDtheta(tht);
        // store function
        f.push_back(1.0 - pl/r);
        // store partial derivatives
        vector <double> row;
        drDtht = DrDtheta(tht);
        // df/dx
        row.push_back( -( x * r/pl - drDtht * dThtDx * pl )/(r * r) );
        // df/dy
        row.push_back( -( y * r/pl - drDtht * dThtDy * pl )/(r * r) );
        // df/dr
        row.push_back( pl/(r * r) );
        dF.push_back(row);
    }

    // bubble sort, not really efficient
    // -but acceptable for such small arrays
    for(int i = 0; i < Get_q()-1; i++) {
        for (int j = i + 1; j < Get_q(); j++) {
            if (f[i] < f[j]) {
                // swap values of f[i] and f[j]
                swap(f[i],f[j]);
                // swap rows dF[i] and dF[j]
                dF[i].swap(dF[j]);
            }
        }
    }

    // compute resulting Rfunction
    // vector for df/dxi
    vector<double> dF1;
    // iterative evaluation of:
    // -the resulting R-functions
    // -the associated partial derivatives
    double f1,fDum;
    // first value of f
    f1 = f[0];
    // first associated row with partial derivatives
    dF1 = dF[0];

    // combine functions as (...((F1 v F2) v F3 ) v F4) v ...)
    // for all intersections
    for(int i = 1; i < Get_q(); i++)
    {
        // compute R-function, sets all partial derivatives
        // fDum and dDum temporary results
        // -of the union from F1 to Fi
        RpUnion(f1, f[i], dF1, dF[i], fDum, dDum);
        // update results in f1 and dF1, and iterate
        f1 = fDum;
        dF1 = dDum;
    }

    // final partial derivatives df/dxi after R-functions
    dfFinal = dF1;
    // clear arrays
    f.clear();    dF.clear();    dDum.clear();    dF1.clear();
    // return results
    return f1;
}

double RationalSuperShape2D::ImplicitFunction3( const Vector2d &point, vector <double> &dfFinal ) {

    dfFinal.clear();

    // nothing computable, return zero values, zero partial derivatives
    // the point will have no effect on the ongoing computations
    if ( point[0] == 0 && point[1] == 0)
    {
        // dF/Dx, dF/Dy, dF/Dr set to zero...
        for (int i = 0; i < 3; i++) {
            dfFinal.push_back(0);
        }
        return 0;
    }

    vector <double> f, dDum;
    double x(point[0]), y(point[1]), PSL(point.squaredNorm()), dThtDx (-y/PSL), dThtDy (x/PSL), R,drDtht;

    vector< vector<double> > dF;

    // assert angular values between [0, 2q*Pi]
    double tht (atan2(y,x)), thtBase(tht);
    if (tht < 0) {
        thtBase += 2.0 * RATIONAL_PI;
    }

    // compute all intersections and associated gradient values
    for (int i = 0; i < Get_q(); i++)
    {
        tht = thtBase + i * 2.0 * RATIONAL_PI;
        R = Radius(tht);
        drDtht = DrDtheta(tht);
        // store function
        f.push_back(log( R*R / PSL));
        // store partial derivatives
        drDtht = DrDtheta(tht);
        vector <double> row;
        // df/dx
        row.push_back( -2.0*(x * R - PSL * drDtht * dThtDx)/(R * PSL)  );
        // df/dy
        row.push_back( -2.0*(y * R - PSL * drDtht * dThtDy)/(R * PSL) );
        // df/dr
        row.push_back( 2.0/R );
        dF.push_back(row);
    }

    // bubble sort, not really efficient
    // -but acceptable for such small arrays
    for(int i = 0; i < Get_q()-1; i++) {
        for (int j = i+1; j < Get_q(); j++) {
            if (f[i] < f[j])
            {
                // swap values of f[i] and f[j]
                swap(f[i],f[j]);
                // swap rows dF[i] and dF[j]
                dF[i].swap(dF[j]);
            }
        }
    }

    // compute resulting Rfunction
    // vector for df/dxi
    vector<double> dF1;

    // iterative evaluation of:
    // -the resulting R-functions
    // -the associated partial derivatives
    double f1,fDum;
    // first value of f
    f1 = f[0];
    // first associated row with partial derivatives
    dF1 = dF[0];

    // combine functions as (...((F1 v F2) v F3 ) v F4) v ...)
    // for all intersections
    for(int i = 1; i < Get_q(); i++)
    {
        // compute R-function, sets all partial derivatives
        // fDum and dDum temporary results of the union from F1 to Fi
        RpUnion(f1, f[i], dF1, dF[i], fDum, dDum);
        // update results in f1 and dF1, and iterate
        f1 = fDum;
        dF1 = dDum;
    }

    // final partial derivatives df/dxi after R-functions
    dfFinal = dF1;
    // clear arrays
    f.clear();    dF.clear();    dDum.clear();    dF1.clear();
    // return results
    return f1;
}

double RationalSuperShape2D::DrDa(const double tht) {
    double c = fabs( cos( Get_p() * tht * 0.25 / Get_q())) ;
    double s = fabs( sin( Get_p() * tht * 0.25 / Get_q())) ;
    double aCn2 = pow(c, Get_n2())/Get_a();
    double bSn3 = pow(s, Get_n3())/Get_b();

    double result1 (aCn2 * pow( aCn2 + bSn3, -1.0/Get_n1() - 1.0));
    double result2 (Get_n1() * Get_a());

    return result1 / result2;
}

double RationalSuperShape2D::DrDb(const double tht) {
    double c = fabs( cos( Get_p() * tht * 0.25 / Get_q())) ;
    double s = fabs( sin( Get_p() * tht * 0.25 / Get_q())) ;

    double aCn2 = pow(c, Get_n2())/Get_a();
    double bSn3 = pow(s, Get_n3())/Get_b();

    double result1 (bSn3 * pow( aCn2 + bSn3, -1.0/Get_n1() - 1.0));
    double result2 (Get_n1() * Get_b());

    return result1 / result2;
}

double RationalSuperShape2D::DrDn1(const double tht) {
    double r(Radius(tht));
    double rn1 = pow(r, -Get_n1());

    if (r > RATIONAL_EPSILON){
        return log(rn1) * r / (Get_n1() * Get_n1());
    }

    return 0;
}

double RationalSuperShape2D::DrDn2(const double tht) {
    double c = fabs( cos( Get_p() * tht * 0.25 / Get_q())) ;
    double s = fabs( sin( Get_p() * tht * 0.25 / Get_q())) ;

    double aCn2 = pow(c, Get_n2())/Get_a();
    double bSn3 = pow(s, Get_n3())/Get_b();

    if(c < RATIONAL_EPSILON) {
        return 0;
    }

    double result1 (-pow( aCn2 + bSn3, -1.0/Get_n1() - 1.0));
    double result2 (log(c) * aCn2 / Get_n1());

    return (result1 * result2);
}

double RationalSuperShape2D::DrDn3(const double tht) {
    double c (fabs( cos( Get_p() * tht * 0.25 / Get_q()))) ;
    double s (fabs( sin( Get_p() * tht * 0.25 / Get_q()))) ;

    double aCn2 = pow(c, Get_n2())/Get_a();
    double bSn3 = pow(s, Get_n3())/Get_b();

    if (s < RATIONAL_EPSILON) {
        return 0;
    }

    double result1 (-pow( aCn2 + bSn3, -1.0/Get_n1() - 1.0));
    double result2 (log(s) * bSn3 / Get_n1());

    return (result1 * result2);
}

double RationalSuperShape2D::DrDtheta(const double tht) {
    double n1(Get_n1()), n2(Get_n2()), n3(Get_n3());
    double m(Get_p()),q(Get_q()),a(Get_a()),b(Get_b());
    double c(cos(m * 0.25 * tht / q)), C(fabs(c));
    double s(sin(m * 0.25 * tht / q)), S(fabs(s));

    if(C < RATIONAL_EPSILON || S < RATIONAL_EPSILON) {
        return 0;
    }

    double result1 ((-1.0/n1) * pow(pow(C,n2) / a + pow(S,n3)/b, -1.0/n1 -1.0));
    double result2 (-0.25 * m * pow(C,n2) * (n2 * tan(m * tht * 0.25/q)) / (a*q));
    double result3 (0.25 * m * pow(S,n3) * (n3 / tan(m * tht * 0.25/q)) / (b*q));

    return result1 * (result2 + result3);
}

double RationalSuperShape2D::Radius(double angle) {
    double tmpAngle = Get_p() * angle * 0.25 / Get_q() ;
    double tmp1( cos(tmpAngle) );
    double tmp2( sin(tmpAngle) );

    tmp1 = pow(fabs(tmp1),Get_n2()) / Get_a();
    tmp2 = pow(fabs(tmp2),Get_n3()) / Get_b();

    if( tmp1 + tmp2 !=0 ) {
        return( pow( (tmp1 + tmp2), -1.0/Get_n1() ) );
    }
    else {
        cout << "ERROR RADIUS NULL" << endl;
        return 0;
    }
}

void RationalSuperShape2D::RpUnion(double f1, double f2, vector<double> dF1,
                                   vector<double> dF2, double &f, vector<double> &dF) {
    assert(dF1.size() == dF2.size());
    dF.clear();

    f = f1 + f2 + sqrt(f1 * f1 + f2 * f2);

    // function differentiable
    if (f1 != 0 || f2 != 0) {
        for (unsigned int i = 0; i < dF1.size(); i++) {
            dF.push_back(dF1[i] + dF2[i] + (f1 * dF1[i] + f2 * dF2[i])
                         / sqrt(f1 * f1 + f2 * f2));
        }
    }
    else {
        // function not differentiable, set everything to zero
        for (unsigned int i = 0; i < dF1.size(); i++)
            dF.push_back(0);
    }
}

void RationalSuperShape2D::Optimize(const vector<Vector2d> &data, double &err,
                                    bool normalization, int functionUsed) {
    double newChiSquare, chiSquare(1e15), oldChiSquare(1e15);
    unsigned int i,k;
    bool stop(false);
    double oldParams[] = {0,0,0,0,0,
                          0,0,0,0,0,
                          0,0,0,0,0,
                          0,0,0};
    double lambdaIncr(10);
    double lambda(pow(lambdaIncr, -12));

    MatrixXd alpha, alpha2;
    VectorXd beta, beta2, dJ, oldBeta,  trial, sigma;

    alpha   =   MatrixXd::Zero(5,5);
    alpha2  =   MatrixXd::Zero(5,5);

    beta    =   VectorXd::Zero(5);
    beta2   =   VectorXd::Zero(5);
    dJ      =   VectorXd::Zero(5);
    oldBeta =   VectorXd::Zero(5);
    trial   =   VectorXd::Zero(8);
    sigma   =   VectorXd::Zero(8);

    // derivatives of translation matrix are
    // -constant and can be computed once for all
    for(int itNum = 0; itNum < 1000 && stop == false; itNum++) {
        bool outofBounds(false);
        // store oldparams
        for(i = 0; i < mParameters.size(); i++)
            oldParams[i] = mParameters[i];

        alpha.setZero();
        beta.setZero();
        alpha2.setZero();
        beta2.setZero();

        chiSquare = XiSquare(data,
                             alpha,
                             beta,
                             itNum == 0,    // init x0, y0, and tht0 or not?
                             false,         // robust or not?
                             functionUsed,  // implicitf cuntion1
                             normalization, // activate normalization or not
                             true);         // update vectors

        // add Lambda to diagonal elements
        // -and solve the matrix
        // linearization of Hessian, cf Numerical Recepies
        for(k = 0; k < 5; k++) {
            // multiplicative factor to make diagonal dominant
            alpha(k,k) *= 1.0 + lambda;
            // additive factor to avoid rank deficient matrix
            alpha(k,k) += lambda;
        }

        // solve system
        alpha.ldlt().solveInPlace(beta);
        // coefficients a and b in [0.01, 100]
        outofBounds =  mParameters[0] + beta[0] < 0.01  || mParameters[0] + beta[0] > 1000 ||
                mParameters[1] + beta[1] < 0.01  || mParameters[1] + beta[1] > 1000 ||
                mParameters[2] + beta[2] < 0.1   || mParameters[2] + beta[2] > 1000 ||
                mParameters[3] + beta[3] < 0.001 || mParameters[3] + beta[3] > 1000 ||
                mParameters[4] + beta[4] < 0.001 || mParameters[4] + beta[4] > 1000;

        if( !outofBounds )
        {
            Set_a(mParameters[0] + beta[0]);
            Set_b(mParameters[1] + beta[1]);
            // coefficients n1 in [1., 1000]
            // setting n1<1. leads to strong
            // -numerical instabilities
            Set_n1( mParameters[2] + beta[2] );
            // coefficients n2,n3 in [0.001, 1000]
            Set_n2( mParameters[3] + beta[3]);
            Set_n3( mParameters[4] + beta[4]);
            // coefficients x0 and y0
            // truncate translation to avoid huge gaps
            // beta[5] = min(0.01, max(-0.01, beta[5]));
            // beta[6] = min(0.01, max(-0.01, beta[6]));
            // beta[7] = min(RATIONAL_PI/100.0, max(-RATIONAL_PI/100.0, beta[7]));
            // mParameters[9] += beta[5];
            // mParameters[10] += beta[6];
            // rotational offset tht0
            // mParameters[7] += beta[7];
        }

        // evaluate chisquare with new values
        oldChiSquare = chiSquare;
        newChiSquare = XiSquare(data,
                                alpha2,
                                beta2,
                                false,         // init x0, y0, and tht0 or not?
                                false,         // robust or not?
                                functionUsed,  // implicitf cuntion1
                                normalization, // actiate normalization or not
                                false);

        // check if better result
        // if new result is not right
        // -restore old params and try
        // -with lambda 10 times bigger
        if( newChiSquare > (0.999 * oldChiSquare) ) {
            lambda *= lambdaIncr;
            for(i = 0; i < mParameters.size(); i++) {
                mParameters[i] = oldParams[i];
            }
        }
        else {
            // successful iteration

            // huge improvement, something may have been wrong
            // this may arise during the first iterations
            // n1 may literally explode, or tend to 0...
            // in such case, the next iteration is successful but leads to a local minimum
            // ==> it is better to verify the result with a smaller step
            // if indeed it was a correct iteration, then it will pass the next time

            // 99% improvement, impossible
            if (newChiSquare <= 0.01 * oldChiSquare) {
                // reduce the step within the search direction
                lambda *= lambdaIncr;
                for(i = 0; i < mParameters.size(); i++) {
                    // restore old parameters
                    mParameters[i] = oldParams[i];
                }
            }
            else {
                // correct and realistic improvement
                lambda /= lambdaIncr;
            }
        }
        // very small displacement ==> local convergence
        if( lambda > 1e15 || newChiSquare < 1e-5) {
            stop = true;
        }
    }	//end of for
    err = chiSquare;
}

void RationalSuperShape2D :: RobustInit(const vector< Vector2d >& data){

    vector <Vector2d> newData;
    vector < int > hist;
    Vector2d T(0,0);
    double r(0);
    bool stop(false);
    double histAvg, oldHistAvg;

    // copy input to newData
    for(unsigned int i = 0; i < data.size(); i++) {
        newData.push_back(data[i]);
    }
    // arbitrary number of iterations
    for(int it = 0; it < 30 && !stop; it++)
    {
        // clean angular histogram
        hist.clear();
        for(int i = 0; i < Get_p(); i++) {
            hist.push_back(0);
        }
        // init position and avg radius
        T << 0,0;
        r = 0;
        // compute center and fill angular histogram
        for (unsigned int i = 0; i < newData.size(); i++)
        {
            Vector2d point(newData[i]);
            double angle = atan2(point[1], point[0]);

            T += point;

            if(angle < 0) {
                angle += 2 * RATIONAL_PI;
            }

            for(unsigned int index = 0; index < hist.size(); index++){
                if((index * (2 * RATIONAL_PI)/Get_p()) <= angle &&
                        (angle < (index+1) * (2 * RATIONAL_PI)/Get_p())) {

                    hist[index] =  hist[index]+1;
                }
            }
        }

        T /= newData.size();

        r = 0;
        for (unsigned int i = 0; i < newData.size(); i++){
            r += (newData[i] - T).norm() ;
        }
        r /=newData.size();

        // compute AVG value for HIST
        oldHistAvg = histAvg;
        histAvg = 0;

        for(unsigned int i = 0; i < hist.size(); i++) {
            histAvg += hist[i];
        }

        histAvg /= hist.size();

        if (histAvg == oldHistAvg) {
            stop = true;
        }

        // now fill data for angular sectors inferior to AVG
        for(unsigned int i = 0; i < hist.size(); i++) {
            // go through angular sectors
            double aMin = i * 2 * RATIONAL_PI/Get_p();

            for(int j = 0; j < (histAvg - hist[i]); j++) {
                // generate a random angle in [amin, amax];
                double angle = aMin +  RATIONAL_PI/Get_p();
                // R1.uniform()*2*PI/(n*Get_p());
                Vector2d flood(-T[0] + r * cos(angle), -T[1] + r * sin(angle));
                newData.push_back(flood);
            }
        }
    }

    Set_xoffset(T[0]);
    Set_yoffset(T[1]);
    Set_a(r);
    Set_b(r);
}

double RationalSuperShape2D::XiSquare(const vector < Vector2d > &data, MatrixXd &alpha,
                                      VectorXd &beta, bool initOn,
                                      bool robust, int functionUsed,
                                      bool normalization, bool update) {
    VectorXd dj;
    vector<double> dF;

    Matrix3d trMatrix;  // translational matrix
    Matrix3d rotMatrix; // rotational matrix
    Matrix3d dTrDx0;    // partial derivative of the translational matrix regarding x
    Matrix3d dTrDy0;    // partial derivative of the translational matrix regarding y
    Matrix3d dRotDtht0; // partial derivative of rotational matrix regarding theta

    unsigned int i,j,k;

    double r, x, y, tht, dThtDx, dThtDy,dThtDx0, dThtDy0, dRdTht,
            chiSquare(1e15),
            f(0),h(0),
            x0(Get_xoffset()),y0(Get_yoffset()),tht0(Get_thtoffset()),
            dXdX0,dXdY0,dXdTht0, dYdX0,dYdY0,dYdTht0, dThtdTht0;

    // derivatives of translation matrix are
    // -constant and can be computed once for all
    dTrDx0 << 0, 0, -1,
            0, 0, 0,
            0, 0, 1;
    dTrDy0 << 0, 0, 0,
            0, 0, -1,
            0, 0, 1;

    dj = VectorXd::Zero(8);

    // functions pointer
    double (RationalSuperShape2D ::*pt2ConstMember)( const Vector2d &P, vector <double> &DfFinal ) = NULL;

    switch (functionUsed)
    {
    case 1 : {
        pt2ConstMember = &RationalSuperShape2D :: ImplicitFunction1;
    } break;

    case 2 :{
        pt2ConstMember = &RationalSuperShape2D :: ImplicitFunction2;
    } break;

    case 3 : {
        pt2ConstMember = &RationalSuperShape2D :: ImplicitFunction3;
    } break;

    default :
        pt2ConstMember = &RationalSuperShape2D :: ImplicitFunction1;
    }

    if(initOn)
    {
        // THIS IS ONLY CALLED AT THE VERY FIRST ITERATION
        if(robust) {
            RobustInit(data);
        }
        else
        {
            // Finding the center of gravity
            Vector2d center(0,0);
            for(i = 0; i < data.size(); i++){
                center += data[i];
            }

            center /= data.size();

            double aMin(1e30);
            for(i = 0; i < data.size(); i++) {
                aMin = min(aMin, (data[i]-center).norm());
            }
            Set_a(aMin);
            Set_b(aMin);
            Set_xoffset(center[0]);
            Set_yoffset(center[1]);
        }
    }

    // clean memory
    if(update) {
        alpha.setZero();
        beta.setZero();
    }

    // evaluate ChiSquare, components
    // -for the beta and matrix alpha
    chiSquare = 0;

    // first define inverse translation T-1
    trMatrix.setZero();
    rotMatrix.setZero();
    dRotDtht0.setZero();

    trMatrix << 1 , 0 , -x0 ,
            0 , 1 , -y0 ,
            0 , 0 , 1;

    // get partial derivatives
    // then define inverse rotation,
    // -i.e. transposed rotation
    rotMatrix << cos(tht0) , sin(tht0) , 0 ,
            -sin(tht0) , cos(tht0) , 0 ,
            0 , 0 , 1;

    // get partial derivatives
    dRotDtht0 << -sin(tht0) , cos(tht0) , 0 ,
            -cos(tht0) , -sin(tht0) , 0 ,
            0 , 0 , 1;

    for(i = 0; i < data.size(); i++)
    {
        double dfDr;

        // global inverse transform is T * R
        Vector2d dum(data[i]);
        Vector3d dum2(dum[0],dum[1],1);
        // apply inverse transform
        Vector3d dum3 ( rotMatrix * (trMatrix * dum2));
        // get partial derivatives for canonical point
        Vector3d dPdX0 ( rotMatrix * (dTrDx0 * dum2) );
        Vector3d dPdY0 ( rotMatrix * (dTrDy0 * dum2) );
        Vector3d dPdTht0 (dRotDtht0 * (trMatrix * dum2) );
        // the canonical point
        Vector2d P(dum3[0],dum3[1]);
        // simplify notations
        dXdX0 = dPdX0[0];
        dXdY0 = dPdY0[0];
        dXdTht0 =  dPdTht0[0];

        dYdX0 = dPdX0[1];
        dYdY0 = dPdY0[1];
        dYdTht0 =  dPdTht0[1];

        x = P[0];
        y = P[1];

        // avoid division by 0 ==> numerical stability

        if (P.norm() < RATIONAL_EPSILON) {
            // avoids division by zero
            continue;
        }

        tht = atan2(y,x);
        if(tht < 0) {
            tht += 2.0 * RATIONAL_PI;
        }

        r = Radius(tht);

        // theta = Arctan(Y/X)
        // -y / (x*x+y*y);
        dThtDx = -sin(tht) ;
        // x / (x*x+y*y);
        dThtDy =  cos(tht);

        // partial derivatives of theta regarding
        // - x offset, y offset, and angular offset
        dThtDx0   = dThtDx * dXdX0  + dThtDy * dYdX0;
        dThtDy0   = dThtDx * dXdY0  + dThtDy * dYdY0;
        dThtdTht0 = dThtDx * dXdTht0 + dThtDy * dYdTht0;

        // avoid non differentiable cases
        dRdTht = DrDtheta(tht);
        // call to the implicit function
        f = (*this.*pt2ConstMember)(P, dF);

        // compute elements beta[i][0] and alpha[i][j]
        // ==> requires partial derivatives!!
        // dF/dR stored at index 2 in array Df
        // -during the call to ImplicitFunction1-2-3
        dfDr = dF[2];

        // F1 = R-PL ==> DfDr = 1. ;
        // F2 = 1-PL/R ==> DfDr = PL/R ;
        // F3 =  log ( R/PSL) ==> DfDr = 2/R
        dj.setZero();
        // df/da = df/dr * dr/da
        dj[0] = dfDr * DrDa(tht) ;
        // df/db = df/dr * dr/db
        dj[1] = dfDr * DrDb(tht) ;
        // df/dn1 = df/dr * dr/dn1
        dj[2] = dfDr * DrDn1(tht);
        // df/dn3 = df/dr * dr/dn3
        dj[4] = dfDr * DrDn3(tht);
        // df/dn2 = df/dr * dr/dn2
        dj[3] = dfDr * DrDn2(tht);
        // df/dx0 = df/dr * dr/dtht *dtht/dx0
        //dj[5]= dfDr * dRdTht*dThtDx0;
        // df/dy0 = df/dr * dr/dtht *dtht/dy0
        //dj[6]= dfDr * dRdTht*dThtDy0;
        // df/dth0 = dfdr * dr/dtht * dtht/dtht0
        //dj[7]= dfDr * dRdTht*dThtdTht0;

        // df/dx + df/dy
        double nablamagn =  dF[0]*dF[0]
                + dF[1]*dF[1];

        h = f * pow(f*f + nablamagn,-0.5);

        if(normalization) {
            chiSquare += h*h;
        }
        else {
            chiSquare += f*f;
        }

        // scale factor for the gradient
        // -if normalizing the function
        if(update) {
            if(normalization) {
                if (fabs(f) > RATIONAL_EPSILON || nablamagn > RATIONAL_EPSILON ) {
                    for (int idum = 0; idum < 5; idum++){
                        dj[idum] *= h * (1.0 - h * h) / f;
                    }
                }
                else {
                    dj.setZero();
                }
            }
            // summation of values for the gradient
            if(normalization) {
                if (f != 0) {
                    for(k = 0; k < 5; k++) {
                        beta[k] -=  h * dj[k];
                    }
                }
            }
            else {
                for(k = 0; k < 5; k++) {
                    beta[k] -= f * dj[k];
                }
            }
            // compute approximation of Hessian matrix
            for(k = 0; k < 5; k++) {
                for(j = 0; j < 5; j++) {
                    // Hessian Calculation
                    alpha(k,j) +=  dj[k] * dj[j];
                }
            }
        }
    }

    return chiSquare;
}

vector< Vector2d > RationalSuperShape2D::Run(const vector<Vector2d> &data,
                                             const vector< float > &rotOffsets,
                                             bool normalization, int functionUsed) {
    mError = 1e30;
    int sizeOffsets = rotOffsets.size();
    for (int j = 0; j < sizeOffsets; j++) {
        for (int i = 1; i < 5; i++) {
            double err = 1e30;
            RationalSuperShape2D tmp;

            int ptmp;
//             if (i==0) ptmp = 1;
//             if (i==1) ptmp = 3;
//             if (i==2) ptmp = 4;
//             if (i==3) ptmp = 6;
//             if (i==4) ptmp = 8;

            //if (i==0) ptmp = 3;
            if (i==1) ptmp = 4;
            if (i==2) ptmp = 6;
            if (i==3) ptmp = 8;

            tmp.Init(1,1,       // and b
                     1,2,2,     // n1  to n3
                     ptmp,1,    // and q
                     rotOffsets[j],0,       // change thtoffset to manual PI/2
                     0,0,0);

            tmp.Optimize(data, err, normalization, functionUsed);

            // less error, so update parameters!
            if (err < mError) {
                Init(tmp.Get_a(), tmp.Get_b(),
                     tmp.Get_n1(), tmp.Get_n2(), tmp.Get_n3(),
                     tmp.Get_p(), tmp.Get_q(),
                     tmp.Get_thtoffset(), tmp.Get_phioffset(),
                     tmp.Get_xoffset(), tmp.Get_yoffset(), tmp.Get_zoffset());
                mError = err;
            }
        }
    }

    vector< Vector2d > output;
    double centerX = Get_xoffset();
    double centerY = Get_yoffset();
    double ro = Get_thtoffset();
    for (double i = 0.0f; i <= 6.26; i += 0.01) {
        double r = Radius(i);       
        double x = cos(i + ro)*r + centerX;
        double y = sin(i + ro)*r + centerY;
        output.push_back(Vector2d(x,y));
    }
    return output;
}
