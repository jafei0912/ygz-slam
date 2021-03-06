#ifndef YGZ_CERES_REPROJECTIONERROR_POINTONLY_H_
#define YGZ_CERES_REPROJECTIONERROR_POINTONLY_H_

#include <ceres/rotation.h>

#include "ygz/Basic.h"
#include "ygz/Algorithm/CVUtils.h"

// 我也真是把ceres玩出花的男人

namespace ygz 
{
    
class CeresReprojectionErrorPointOnly
{
public:
    EIGEN_MAKE_ALIGNED_OPERATOR_NEW 
    CeresReprojectionErrorPointOnly( 
        const Vector2d& pt_cam, const SE3& TCW
    ): _pt_cam(pt_cam)  
    {
        // 前面三项为t，后面三项为r 
        _TCW.head<3>() = TCW.translation();
        _TCW.tail<3>() = TCW.so3().log();
    }
    
    void SetWeight( const double& weight )
    {
        _weight = weight;
    }
    
    void Enable( bool enable=true ) 
    {
        _enable = true; 
    }
    
    // cost = z - (Rp+t), note the first three components in the pose are translation 
    template< typename T > 
    bool operator() ( 
        const T* const point_world, 
        T* residuals
    ) const {
        
        if ( _enable == false )
        {
            residuals[0] = residuals[1] = 0;
            return true;
        }
        
        T p[3];
        T rot[3];
        for ( size_t i=0; i<3; i++ )
            rot[i] = (T) _TCW[i+3];
        ceres::AngleAxisRotatePoint<T>( rot, point_world, p );
        
        p[0] += (T) _TCW[0]; 
        p[1] += (T) _TCW[1]; 
        p[2] += (T) _TCW[2]; 
        
        if ( p[2] < T(0) ) {
            residuals[0] = residuals[1] = T(0);
            return false;
        }
        
        residuals[0] = _pt_cam[0] - p[0]/p[2];
        residuals[1] = _pt_cam[1] - p[1]/p[2];
        return true; 
    }
    
private:
    Vector2d _pt_cam; // observation: normalized camera coordinate 
    Vector6d _TCW; // 3D point in world frame 
    double _weight =1;
    bool _enable = true;
};
    
}

#endif 