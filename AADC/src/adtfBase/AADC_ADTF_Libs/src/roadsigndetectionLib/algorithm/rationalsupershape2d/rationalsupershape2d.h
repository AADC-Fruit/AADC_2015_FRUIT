#ifndef RATIONALSUPERSHAPE2D_H
#define RATIONALSUPERSHAPE2D_H

#include <cstring>

#include "irationalsupershape2d.h"

#define RATIONAL_PI         3.14159265358979323846
#define RATIONAL_EPSILON    1e-9

class RationalSuperShape2D: public IRationalSuperShape2D
{
    public:
        RationalSuperShape2D();
        RationalSuperShape2D(double a, double b, double n1,
                             double n2,double n3,double p,
                             double q, double thtOffset=0, double phiOffset=0,
                             double xOffset=0, double yOffset=0, double zOffset=0);
        ~RationalSuperShape2D();

        void Init( double a, double b, double n1,
                   double n2,double n3,double p,
                   double q, double thtOffset=0,
                   double phiOffset=0, double xOffset=0,
                   double yOffset=0, double zOffset=0);
        void Init( double a, double b, double n1,double n2,double n3);

        double ImplicitFunction1( const Vector2d &point, vector <double> &dfFinal );
        double ImplicitFunction2( const Vector2d &point, vector <double> &dfFinal );
        double ImplicitFunction3( const Vector2d &point, vector <double> &dfFinal );

        double DrDa(const double tht);
        double DrDb(const double tht);
        double DrDn1(const double tht);
        double DrDn2(const double tht);
        double DrDn3(const double tht);
        double DrDtheta(const double tht);

        void Optimize(const vector< Vector2d > &data, double &err ,
                       bool normalization = true,
                       int functionused = 1);

        void RobustInit(const vector< Vector2d >& data);

        double Radius(double angle);

        double XiSquare(const vector < Vector2d > &data, MatrixXd &alpha,
                                                VectorXd &beta, bool initOn = false,
                                                bool robust =  true, int functionUsed = 1,
                                                bool normalization = false, bool update = false);

        vector < Vector2d > Run(const vector<Vector2d> &data,
								const vector< float > &rotOffsets,
                                bool normalization=true, int functionUsed=1);

        inline Vector2d Point(double angle) {
            double r = Radius(angle);
            return Vector2d(r*cos(angle),r*sin(angle));
        }

        inline double Get_a()  {return mParameters [0];}
        inline double Get_b()  {return mParameters [1];}

        inline double Get_n1() {return mParameters [2];}
        inline double Get_n2() {return mParameters [3];}
        inline double Get_n3() {return mParameters [4];}

        inline double Get_p()  {return mParameters [5];}
        inline double Get_q()  {return mParameters [6];}

        inline double Get_thtoffset()  {return mParameters [7];}
        inline double Get_phioffset()  {return mParameters [8];}

        inline double Get_xoffset()  {return mParameters [9];}
        inline double Get_yoffset()  {return mParameters [10];}
        inline double Get_zoffset()  {return mParameters [11];}

        inline void Set_a ( const double a)  {mParameters [0]=fabs(a);}
        inline void Set_b ( const double b)  {mParameters [1]=fabs(b);}

        inline void Set_n1( const double n1) {mParameters [2]=fabs(n1);}
        inline void Set_n2( const double n2) {mParameters [3]=fabs(n2);}
        inline void Set_n3( const double n3) {mParameters [4]=fabs(n3);}

        inline void Set_p ( const double p)  {mParameters [5]=p;}
        inline void Set_q ( const double q)  {mParameters [6]=q;}

        inline void Set_thtoffset ( const double thtoffset)  {mParameters [7]=thtoffset;}
        inline void Set_phioffset ( const double phioffset)  {mParameters [8]=phioffset;}

        inline void Set_xoffset ( const double xoffset)  {mParameters [9]=xoffset;}
        inline void Set_yoffset ( const double yoffset)  {mParameters [10]=yoffset;}
        inline void Set_zoffset ( const double zoffset)  {mParameters [11]=zoffset;}

        void RpUnion(double f1, double f2, vector<double> Df1,
                     vector<double> Df2, double &f, vector<double> &Df);

    private:
        // mParameters of the rational supershape
        // mParameters[0] : a
        // mParameters[1] : b

        // mParameters[2] : n1
        // mParameters[3] : n2
        // mParameters[4] : n3

        // mParameters[5] : p    Team U: no .of sectors of 2*pi . 2*pi/p= smallest step increment (see RobustInit())
        // mParameters[6] : q

        // mParameters[7] : theta offset
        // mParameters[8] : phi offset // unused in 2D

        // mParameters[9] : x offset
        // mParameters[10] : y offset
        // mParameters[11] : z offset // unused in 2D
        vector<double> mParameters;
        double mError;
};

#endif // RATIONALSUPERSHAPE2D_H
