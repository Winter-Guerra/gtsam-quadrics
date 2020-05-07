/* ----------------------------------------------------------------------------

 * QuadricSLAM Copyright 2020, ARC Centre of Excellence for Robotic Vision, Queensland University of Technology (QUT)
 * Brisbane, QLD 4000
 * All Rights Reserved
 * Authors: Lachlan Nicholson, et al. (see THANKS for the full author list)
 * See LICENSE for the license information

 * -------------------------------------------------------------------------- */

/**
 * @file main.cpp
 * @date Apr 16, 2020
 * @author Lachlan Nicholson
 * @brief a simple main function to facilitate debugging
 */

#include <quadricslam/geometry/ConstrainedDualQuadric.h>
#include <quadricslam/geometry/BoundingBoxFactor.h>
#include <quadricslam/geometry/QuadricCamera.h>
#include <quadricslam/geometry/AlignedBox2.h>
#include <quadricslam/geometry/DualConic.h>
#include <quadricslam/base/TestClass.h>

#include <gtsam/geometry/Cal3_S2.h>
#include <gtsam/inference/Symbol.h>
#include <gtsam/geometry/Pose2.h>

using namespace std;
using namespace gtsam;

int main() {

  // DualConic conic(Pose2(Rot2(), Point2(45.2,13.8)), (Vector2() << 1.0,3.0).finished());
  // DualConic conicNorm(conic.matrix()/conic.matrix()(2,2));
  // cout << "conic polynomial\n" << conic.polynomial() << endl << endl;
  // cout << "conic normalized poly\n" << conicNorm.polynomial() << endl << endl;
  // cout << "conic matrix\n" << conic.matrix() << endl << endl;
  // cout << "conicNorm matrix\n" << conicNorm.matrix() << endl << endl;
  // cout << "conic matrix.inverse()\n" << conic.matrix().inverse() << endl << endl;
  // cout << "conicNorm matrix.inverse()\n" << conicNorm.matrix().inverse() << endl << endl;

  // create measurement, calibration, dimensions, keys, model, pose, quadric
  AlignedBox2 measured(15.2, 18.5, 120.5, 230.2);
  boost::shared_ptr<Cal3_S2> calibration(new Cal3_S2(525.0, 525.0, 0.0, 320.0, 240.0));
  Vector2 imageDimensions(320.0, 240.0);
  Key poseKey(Symbol('x', 1));
  Key quadricKey(Symbol('q', 1));
  boost::shared_ptr<noiseModel::Diagonal> model = noiseModel::Diagonal::Sigmas(Vector4(0.2,0.2,0.2,0.2));

  // set camera pose and quadric
  Pose3 cameraPose = Pose3::Retract((Vector6() << 1.1,2.2,3.3, 4.4,-5.5,-10.0).finished());
  ConstrainedDualQuadric quadric = ConstrainedDualQuadric::Retract((Vector9() << 1.1,2.2,3.3,4.4,5.5,6.6,7.7,8.8,9.9).finished());
  // Pose3 cameraPose(Rot3(), Point3(0,0,-3));
  // ConstrainedDualQuadric quadric;

  // create and use bbf
  BoundingBoxFactor bbf(measured, calibration, imageDimensions, poseKey, quadricKey, model);

  // use bbf to evaluate error with jacobians
  Matrix db_dx;
  Matrix db_dq;
  Vector4 error = bbf.evaluateError(cameraPose, quadric, db_dx, db_dq);
  cout << "error: " << error.transpose() << endl;
  cout << "db_dx\n" << db_dx << endl;
  cout << "db_dq\n" << db_dq << endl;
  // return 1;


  // define expression for x,q
  Expression<Pose3> cameraPose_('x',1);
  Expression<ConstrainedDualQuadric> quadric_('q',1);

  // create values
  Values values;
  values.insert(symbol('x',1), cameraPose);
  values.insert(symbol('q',1), quadric);

  // create gradients
  std::vector<Matrix> gradients;
  Eigen::Matrix<double, 4,6> db_dx1;
  Eigen::Matrix<double, 4,9> db_dq1;
  gradients.push_back(db_dq1);
  gradients.push_back(db_dx1);

  // // request expression and jacobians
  Expression<AlignedBox2> bbfExpression = bbf.expression(cameraPose_, quadric_);
  AlignedBox2 result = bbfExpression.value(values, gradients);

  // // extract gradients
  db_dq1 = gradients[0];
  db_dx1 = gradients[1];

  // print gradients
  result.print("bounds");
  cout << "db_dx1\n" << db_dx1 << endl;
  cout << "db_dq1\n" << db_dq1 << endl;

  cout << "done" << endl;
  return 1;
}

