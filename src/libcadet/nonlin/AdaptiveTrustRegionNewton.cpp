// =============================================================================
//  CADET - The Chromatography Analysis and Design Toolkit
//  
//  Copyright © 2008-2017: The CADET Authors
//            Please see the AUTHORS and CONTRIBUTORS file.
//  
//  All rights reserved. This program and the accompanying materials
//  are made available under the terms of the GNU Public License v3.0 (or, at
//  your option, any later version) which accompanies this distribution, and
//  is available at http://www.gnu.org/licenses/gpl.html
// =============================================================================

#include "nonlin/AdaptiveTrustRegionNewton.hpp"
#include "cadet/ParameterProvider.hpp"
#include "linalg/DenseMatrix.hpp"

namespace cadet
{

namespace nonlin
{

AdaptiveTrustRegionNewtonSolver::AdaptiveTrustRegionNewtonSolver() : _initDamping(1e-2), _minDamping(1e-4), _maxIter(50) { }
AdaptiveTrustRegionNewtonSolver::~AdaptiveTrustRegionNewtonSolver() { }

bool AdaptiveTrustRegionNewtonSolver::configure(IParameterProvider& paramProvider)
{
	if (paramProvider.exists("INIT_DAMPING"))
		_initDamping = paramProvider.getDouble("INIT_DAMPING");
	if (paramProvider.exists("MIN_DAMPING"))
		_minDamping = paramProvider.getDouble("MIN_DAMPING");
	if (paramProvider.exists("MAX_ITERATIONS"))
		_maxIter = paramProvider.getInt("MAX_ITERATIONS");
	return true;
}

bool AdaptiveTrustRegionNewtonSolver::solve(std::function<bool(double const* const, double* const)> residual, std::function<bool(double const* const, linalg::detail::DenseMatrixBase& jac)> jacobian,
		double tol, double* const point, double* const workingMemory, linalg::detail::DenseMatrixBase& jacMatrix, unsigned int size) const
{
	return adaptiveTrustRegionNewtonMethod(residual, [&](double const* const x, double* const y) -> bool {
			return jacobian(x, jacMatrix) && jacMatrix.factorize() && jacMatrix.solve(y);
		},
		_maxIter, tol, _initDamping, _minDamping, point, workingMemory, size);
}


RobustAdaptiveTrustRegionNewtonSolver::RobustAdaptiveTrustRegionNewtonSolver() : _initDamping(1e-2), _minDamping(1e-4), _maxIter(50) { }
RobustAdaptiveTrustRegionNewtonSolver::~RobustAdaptiveTrustRegionNewtonSolver() { }

bool RobustAdaptiveTrustRegionNewtonSolver::configure(IParameterProvider& paramProvider)
{
	if (paramProvider.exists("INIT_DAMPING"))
		_initDamping = paramProvider.getDouble("INIT_DAMPING");
	if (paramProvider.exists("MIN_DAMPING"))
		_minDamping = paramProvider.getDouble("MIN_DAMPING");
	if (paramProvider.exists("MAX_ITERATIONS"))
		_maxIter = paramProvider.getInt("MAX_ITERATIONS");
	return true;
}

bool RobustAdaptiveTrustRegionNewtonSolver::solve(std::function<bool(double const* const, double* const)> residual, std::function<bool(double const* const, linalg::detail::DenseMatrixBase& jac)> jacobian,
		double tol, double* const point, double* const workingMemory, linalg::detail::DenseMatrixBase& jacMatrix, unsigned int size) const
{
	return robustAdaptiveTrustRegionNewtonMethod(residual, [&](double const* const x, double* const y) -> bool {
			return jacobian(x, jacMatrix) && jacMatrix.factorize() && jacMatrix.solve(y);
		},
		[&](double* const y) -> bool {
			return jacMatrix.solve(y);
		},
		_maxIter, tol, _initDamping, _minDamping, point, workingMemory, size);
}


} // namespace nonlin

} // namespace cadet
