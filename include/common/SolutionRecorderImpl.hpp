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

/**
 * @file 
 * Provides several implementations of ISolutionRecorder.
 */

#ifndef LIBCADET_SOLUTIONRECORDER_IMPL_HPP_
#define LIBCADET_SOLUTIONRECORDER_IMPL_HPP_

#include <vector>
#include <sstream>

#include "cadet/SolutionRecorder.hpp"

namespace cadet
{

/**
 * @brief Stores pieces of the solution of one single unit operation in internal buffers
 * @details The pieces of stored solutions are selectable at runtime.
 * @todo Use better storage than std::vector (control growth, maybe chunked storage -> needs chunked writes)
 */
class InternalStorageUnitOpRecorder : public ISolutionRecorder
{
public:

	struct StorageConfig
	{
		bool storeColumn;
		bool storeParticle;
		bool storeFlux;
		bool storeOutlet;
		bool storeInlet;
	};

	InternalStorageUnitOpRecorder() : InternalStorageUnitOpRecorder(UnitOpIndep) { }

	InternalStorageUnitOpRecorder(UnitOpIdx idx) : _cfgSolution({false, false, false, true, false}),
		_cfgSolutionDot({false, false, false, false, false}), _cfgSensitivity({false, false, false, true, false}),
		_cfgSensitivityDot({false, false, false, true, false}), _storeTime(false), _splitComponents(true), _curCfg(nullptr),
		_nComp(0), _numTimesteps(0), _numSens(0), _unitOp(idx), _needsReAlloc(false)
	{
	}

	virtual ~InternalStorageUnitOpRecorder() CADET_NOEXCEPT
	{
		for (unsigned int i = 0; i < _sensOutlet.size(); ++i)
		{
			delete _sensOutlet[i];
			delete _sensInlet[i];
			delete _sensColumn[i];
			delete _sensParticle[i];
			delete _sensFlux[i];

			delete _sensOutletDot[i];
			delete _sensInletDot[i];
			delete _sensColumnDot[i];
			delete _sensParticleDot[i];
			delete _sensFluxDot[i];
		}
	}

	virtual void clear()
	{
		// Clear solution storage
		_time.clear();
		_outlet.clear();
		_inlet.clear();
		_column.clear();
		_particle.clear();
		_flux.clear();

		_outletDot.clear();
		_inletDot.clear();
		_columnDot.clear();
		_particleDot.clear();
		_fluxDot.clear();

		// Clear all sensitivity storage
		for (unsigned int i = 0; i < _sensOutlet.size(); ++i)
		{
			_sensOutlet[i]->clear();
			_sensInlet[i]->clear();
			_sensColumn[i]->clear();
			_sensParticle[i]->clear();
			_sensFlux[i]->clear();

			_sensOutletDot[i]->clear();
			_sensInletDot[i]->clear();
			_sensColumnDot[i]->clear();
			_sensParticleDot[i]->clear();
			_sensFluxDot[i]->clear();
		}
	}

	virtual void prepare(unsigned int numDofs, unsigned int numSens, unsigned int numTimesteps)
	{
		_numTimesteps = numTimesteps;
		_numSens = numSens;

		// Allocate sensitivity storage
		_sensOutlet.resize(numSens, nullptr);
		_sensInlet.resize(numSens, nullptr);
		_sensColumn.resize(numSens, nullptr);
		_sensParticle.resize(numSens, nullptr);
		_sensFlux.resize(numSens, nullptr);

		_sensOutletDot.resize(numSens, nullptr);
		_sensInletDot.resize(numSens, nullptr);
		_sensColumnDot.resize(numSens, nullptr);
		_sensParticleDot.resize(numSens, nullptr);
		_sensFluxDot.resize(numSens, nullptr);

		for (unsigned int i = 0; i < numSens; ++i)
		{
			_sensOutlet[i] = new std::vector<double>();
			_sensInlet[i] = new std::vector<double>();
			_sensColumn[i] = new std::vector<double>();
			_sensParticle[i] = new std::vector<double>();
			_sensFlux[i] = new std::vector<double>();

			_sensOutletDot[i] = new std::vector<double>();
			_sensInletDot[i] = new std::vector<double>();
			_sensColumnDot[i] = new std::vector<double>();
			_sensParticleDot[i] = new std::vector<double>();
			_sensFluxDot[i] = new std::vector<double>();
		}

		_needsReAlloc = false;
	}

	virtual void notifyIntegrationStart(unsigned int numDofs, unsigned int numSens, unsigned int numTimesteps)
	{
		_needsReAlloc = (numSens != _numSens) || (numTimesteps > _numTimesteps);

		// Clear all data from memory
		clear();		

		_numTimesteps = numTimesteps;
		
		if (numSens != _numSens)
		{
			// Delete all sensitivity storage
			for (unsigned int i = 0; i < _sensOutlet.size(); ++i)
			{
				delete _sensOutlet[i];
				delete _sensInlet[i];
				delete _sensColumn[i];
				delete _sensParticle[i];
				delete _sensFlux[i];

				delete _sensOutletDot[i];
				delete _sensInletDot[i];
				delete _sensColumnDot[i];
				delete _sensParticleDot[i];
				delete _sensFluxDot[i];
			}

			// Allocate sensitivity storage
			_sensOutlet.clear();
			_sensOutlet.resize(numSens, nullptr);
			_sensInlet.clear();
			_sensInlet.resize(numSens, nullptr);
			_sensColumn.clear();
			_sensColumn.resize(numSens, nullptr);
			_sensParticle.clear();
			_sensParticle.resize(numSens, nullptr);
			_sensFlux.clear();
			_sensFlux.resize(numSens, nullptr);

			_sensOutletDot.clear();
			_sensOutletDot.resize(numSens, nullptr);
			_sensInletDot.clear();
			_sensInletDot.resize(numSens, nullptr);
			_sensColumnDot.clear();
			_sensColumnDot.resize(numSens, nullptr);
			_sensParticleDot.clear();
			_sensParticleDot.resize(numSens, nullptr);
			_sensFluxDot.clear();
			_sensFluxDot.resize(numSens, nullptr);

			// Populate with empty vectors
			for (unsigned int i = 0; i < numSens; ++i)
			{
				_sensOutlet[i] = new std::vector<double>();
				_sensInlet[i] = new std::vector<double>();
				_sensColumn[i] = new std::vector<double>();
				_sensParticle[i] = new std::vector<double>();
				_sensFlux[i] = new std::vector<double>();

				_sensOutletDot[i] = new std::vector<double>();
				_sensInletDot[i] = new std::vector<double>();
				_sensColumnDot[i] = new std::vector<double>();
				_sensParticleDot[i] = new std::vector<double>();
				_sensFluxDot[i] = new std::vector<double>();
			}

			_numSens = numSens;
		}
	}

	virtual void unitOperationStructure(UnitOpIdx idx, const IModel& model, const ISolutionExporter& exporter)
	{
		// Only record one unit operation
		if (idx != _unitOp)
			return;

		_nComp = exporter.numComponents();

		// Query structure
		unsigned int len = 0;
		StateOrdering const* order = exporter.concentrationOrdering(len);
		_columnLayout.clear();
		_columnLayout.reserve(len + 1); // First slot is time
		_columnLayout.push_back(0);
		for (unsigned int i = 0; i < len; ++i)
		{
			switch (order[i])
			{
				case StateOrdering::Component:
					_columnLayout.push_back(exporter.numComponents());
					break;
				case StateOrdering::AxialCell:
					_columnLayout.push_back(exporter.numAxialCells());
					break;
				case StateOrdering::RadialCell:
				case StateOrdering::Phase:
					break;
			}
		}

		order = exporter.mobilePhaseOrdering(len);
		_particleLayout.clear();
		_particleLayout.reserve(len + 1); // First slot is time
		_particleLayout.push_back(0);
		for (unsigned int i = 0; i < len; ++i)
		{
			switch (order[i])
			{
				case StateOrdering::Component:
					_particleLayout.push_back(exporter.numComponents() + exporter.numBoundStates());
					break;
				case StateOrdering::AxialCell:
					_particleLayout.push_back(exporter.numAxialCells());
					break;
				case StateOrdering::RadialCell:
					_particleLayout.push_back(exporter.numRadialCells());
					 break;
				case StateOrdering::Phase:
					break;
			}
		}

		order = exporter.fluxOrdering(len);
		_fluxLayout.clear();
		_fluxLayout.reserve(len + 1); // First slot is time
		_fluxLayout.push_back(0);
		for (unsigned int i = 0; i < len; ++i)
		{
			switch (order[i])
			{
				case StateOrdering::Component:
					_fluxLayout.push_back(exporter.numComponents());
					break;
				case StateOrdering::AxialCell:
					_fluxLayout.push_back(exporter.numAxialCells());
					break;
				case StateOrdering::RadialCell:
				case StateOrdering::Phase:
					break;
			}
		}
		
		// Everything is ok, we have nothing to do
		if (!_needsReAlloc)
		{
			// Reset for counting the number of received time steps
			_numTimesteps = 0;
			return;
		}

		// Allocate space for solution
		beginSolution();
		allocateMemory(exporter);		
		endSolution();

		beginSolutionDerivative();
		allocateMemory(exporter);		
		endSolution();

		// Allocate space for sensitivities
		for (unsigned int i = 0; i < _sensOutlet.size(); ++i)
		{
			beginSensitivity(i);
			allocateMemory(exporter);
			endSolution();

			beginSensitivityDot(i);
			allocateMemory(exporter);
			endSolution();
		}

		// Reset for counting the number of received time steps
		_numTimesteps = 0;
	}

	virtual void beginTimestep(double t)
	{
		++_numTimesteps;
		if (_storeTime)
			_time.push_back(t);
	}

	virtual void beginUnitOperation(cadet::UnitOpIdx idx, const cadet::IModel& model, const cadet::ISolutionExporter& exporter)
	{
		// Only record one unit operation
		if ((idx != _unitOp) || !_curCfg)
			return;

		unsigned int stride = 0;

		if (_curCfg->storeOutlet)
		{
			double const* outlet = exporter.outlet(stride);
			for (unsigned int i = 0; i < _nComp; ++i)
				_curOutlet->push_back(outlet[i * stride]);
		}

		if (_curCfg->storeInlet)
		{
			double const* inlet = exporter.inlet(stride);
			for (unsigned int i = 0; i < _nComp; ++i)
				_curInlet->push_back(inlet[i * stride]);
		}

		if (_curCfg->storeColumn)
		{
			double const* const data = exporter.concentration();
			_curBulk->insert(_curBulk->end(), data, data + exporter.numColumnDofs());
		}

		if (_curCfg->storeParticle)
		{
			double const* const data = exporter.mobilePhase();
			_curParticle->insert(_curParticle->end(), data, data + exporter.numParticleDofs());
		}

		if (_curCfg->storeFlux)
		{
			double const* const data = exporter.flux();
			_curFlux->insert(_curFlux->end(), data, data + exporter.numFluxDofs());
		}
	}

	virtual void endUnitOperation() { }

	virtual void endTimestep() { }

	virtual void beginSolution()
	{
		_curCfg = &_cfgSolution;
		_curOutlet = &_outlet;
		_curInlet = &_inlet;
		_curBulk = &_column;
		_curParticle = &_particle;
		_curFlux = &_flux;
	}

	virtual void endSolution()
	{
		_curCfg = nullptr;
		_curOutlet = nullptr;
		_curInlet = nullptr;
		_curBulk = nullptr;
		_curParticle = nullptr;
		_curFlux = nullptr;
	}

	virtual void beginSolutionDerivative()
	{
		_curCfg = &_cfgSolutionDot;
		_curOutlet = &_outletDot;
		_curInlet = &_inletDot;
		_curBulk = &_columnDot;
		_curParticle = &_particleDot;
		_curFlux = &_fluxDot;
	}

	virtual void endSolutionDerivative() { endSolution(); }

	virtual void beginSensitivity(const cadet::ParameterId& pId, unsigned int sensIdx)
	{
		beginSensitivity(sensIdx);
	}

	virtual void endSensitivity(const cadet::ParameterId& pId, unsigned int sensIdx)
	{
		endSolution();
	}
	
	virtual void beginSensitivityDerivative(const cadet::ParameterId& pId, unsigned int sensIdx)
	{
		beginSensitivityDot(sensIdx);
	}

	virtual void endSensitivityDerivative(const cadet::ParameterId& pId, unsigned int sensIdx)
	{
		endSolution();
	}

	template <typename Writer_t>
	void writeSolution(Writer_t& writer)
	{
		std::ostringstream oss;

		if (_storeTime)
			writer.template vector<double>("SOLUTION_TIMES", _time.size(), _time.data());

		beginSolution();
		writeData(writer, "SOLUTION", oss);
		endSolution();

		beginSolutionDerivative();
		writeData(writer, "SOLDOT", oss);
		endSolution();
	}

	template <typename Writer_t>
	void writeSensitivity(Writer_t& writer)
	{
		std::ostringstream oss;

		for (unsigned int param = 0; param < _numSens; ++param)
		{
			oss.str("");
			oss << "param_" << std::setfill('0') << std::setw(3) << std::setprecision(0) << param;
			writer.pushGroup(oss.str());

			beginSensitivity(param);
			writeData(writer, "SENS", oss);
			endSolution();

			beginSensitivityDot(param);
			writeData(writer, "SENSDOT", oss);
			endSolution();

			writer.popGroup();
		}
	}

	template <typename Writer_t>
	void writeSensitivity(Writer_t& writer, unsigned int param)
	{
		std::ostringstream oss;

		beginSensitivity(param);
		writeData(writer, "SENS", oss);
		endSolution();

		beginSensitivityDot(param);
		writeData(writer, "SENSDOT", oss);
		endSolution();
	}

	inline StorageConfig& solutionConfig() CADET_NOEXCEPT { return _cfgSolution; }
	inline const StorageConfig& solutionConfig() const CADET_NOEXCEPT { return _cfgSolution; }
	inline void solutionConfig(const StorageConfig& cfg) CADET_NOEXCEPT { _cfgSolution = cfg; }

	inline StorageConfig& solutionDotConfig() CADET_NOEXCEPT { return _cfgSolutionDot; }
	inline const StorageConfig& solutionDotConfig() const CADET_NOEXCEPT { return _cfgSolutionDot; }
	inline void solutionDotConfig(const StorageConfig& cfg) CADET_NOEXCEPT { _cfgSolutionDot = cfg; }

	inline StorageConfig& sensitivityConfig() CADET_NOEXCEPT { return _cfgSensitivity; }
	inline const StorageConfig& sensitivityConfig() const CADET_NOEXCEPT { return _cfgSensitivity; }
	inline void sensitivityConfig(const StorageConfig& cfg) CADET_NOEXCEPT { _cfgSensitivity = cfg; }

	inline StorageConfig& sensitivityDotConfig() CADET_NOEXCEPT { return _cfgSensitivityDot; }
	inline const StorageConfig& sensitivityDotConfig() const CADET_NOEXCEPT { return _cfgSensitivityDot; }
	inline void sensitivityDotConfig(const StorageConfig& cfg) CADET_NOEXCEPT { _cfgSensitivityDot = cfg; }

	inline bool storeTime() const CADET_NOEXCEPT { return _storeTime; }
	inline void storeTime(bool st) CADET_NOEXCEPT { _storeTime = st; }

	inline bool splitComponents() const CADET_NOEXCEPT { return _splitComponents; }
	inline void splitComponents(bool st) CADET_NOEXCEPT { _splitComponents = st; }

	inline UnitOpIdx unitOperation() const CADET_NOEXCEPT { return _unitOp; }
	inline void unitOperation(UnitOpIdx idx) CADET_NOEXCEPT { _unitOp = idx; }

	inline unsigned int numDataPoints() const CADET_NOEXCEPT { return _numTimesteps; }
	inline unsigned int numComponents() const CADET_NOEXCEPT { return _nComp; }

	inline double const* time() const CADET_NOEXCEPT { return _time.data(); }
	inline double const* inlet() const CADET_NOEXCEPT { return _inlet.data(); }
	inline double const* outlet() const CADET_NOEXCEPT { return _outlet.data(); }
	inline double const* column() const CADET_NOEXCEPT { return _column.data(); }
	inline double const* particle() const CADET_NOEXCEPT { return _particle.data(); }
	inline double const* flux() const CADET_NOEXCEPT { return _flux.data(); }
	inline double const* inletDot() const CADET_NOEXCEPT { return _inletDot.data(); }
	inline double const* outletDot() const CADET_NOEXCEPT { return _outletDot.data(); }
	inline double const* columnDot() const CADET_NOEXCEPT { return _columnDot.data(); }
	inline double const* particleDot() const CADET_NOEXCEPT { return _particleDot.data(); }
	inline double const* fluxDot() const CADET_NOEXCEPT { return _fluxDot.data(); }
	inline double const* sensInlet(unsigned int idx) const CADET_NOEXCEPT { return _sensInlet[idx]->data(); }
	inline double const* sensOutlet(unsigned int idx) const CADET_NOEXCEPT { return _sensOutlet[idx]->data(); }
	inline double const* sensColumn(unsigned int idx) const CADET_NOEXCEPT { return _sensColumn[idx]->data(); }
	inline double const* sensParticle(unsigned int idx) const CADET_NOEXCEPT { return _sensParticle[idx]->data(); }
	inline double const* sensFlux(unsigned int idx) const CADET_NOEXCEPT { return _sensFlux[idx]->data(); }
	inline double const* sensInletDot(unsigned int idx) const CADET_NOEXCEPT { return _sensInletDot[idx]->data(); }
	inline double const* sensOutletDot(unsigned int idx) const CADET_NOEXCEPT { return _sensOutletDot[idx]->data(); }
	inline double const* sensColumnDot(unsigned int idx) const CADET_NOEXCEPT { return _sensColumnDot[idx]->data(); }
	inline double const* sensParticleDot(unsigned int idx) const CADET_NOEXCEPT { return _sensParticleDot[idx]->data(); }
	inline double const* sensFluxDot(unsigned int idx) const CADET_NOEXCEPT { return _sensFluxDot[idx]->data(); }
protected:

	inline void beginSensitivity(unsigned int sensIdx)
	{
		_curCfg = &_cfgSensitivity;
		_curOutlet = _sensOutlet[sensIdx];
		_curInlet = _sensInlet[sensIdx];
		_curBulk = _sensColumn[sensIdx];
		_curParticle = _sensParticle[sensIdx];
		_curFlux = _sensFlux[sensIdx];
	}

	inline void beginSensitivityDot(unsigned int sensIdx)
	{
		_curCfg = &_cfgSensitivityDot;
		_curOutlet = _sensOutletDot[sensIdx];
		_curInlet = _sensInletDot[sensIdx];
		_curBulk = _sensColumnDot[sensIdx];
		_curParticle = _sensParticleDot[sensIdx];
		_curFlux = _sensFluxDot[sensIdx];
	}

	inline void allocateMemory(const ISolutionExporter& exporter)
	{
		if (_curCfg->storeOutlet)
			_curOutlet->reserve(std::max(_numTimesteps, 100u) * _nComp);

		if (_curCfg->storeInlet)
			_curInlet->reserve(std::max(_numTimesteps, 100u) * _nComp);
		
		if (_curCfg->storeColumn)
			_curBulk->reserve(std::max(_numTimesteps, 100u) * exporter.numColumnDofs());
		
		if (exporter.hasParticleMobilePhase() && _curCfg->storeParticle)
			_curParticle->reserve(std::max(_numTimesteps, 100u) * exporter.numParticleDofs());
		
		if (exporter.hasParticleFlux() && _curCfg->storeFlux)
			_curFlux->reserve(std::max(_numTimesteps, 100u) * exporter.numFluxDofs());
	}

	template <typename Writer_t>
	void writeData(Writer_t& writer, const char* prefix, std::ostringstream& oss)
	{
		if (_curCfg->storeOutlet)
		{
			if (_splitComponents)
			{
				for (unsigned int comp = 0; comp < _nComp; ++comp)
				{
					oss.str("");
					oss << prefix << "_COLUMN_OUTLET_COMP_" << std::setfill('0') << std::setw(3) << std::setprecision(0) << comp;
					writer.template vector<double>(oss.str(), _numTimesteps, _curOutlet->data() + comp, _nComp);
				}
			}
			else
			{
				oss.str("");
				oss << prefix << "_COLUMN_OUTLET";
				writer.template matrix<double>(oss.str(), _numTimesteps, _nComp, _curOutlet->data(), 1);
			}
		}

		if (_curCfg->storeInlet)
		{
			if (_splitComponents)
			{
				for (unsigned int comp = 0; comp < _nComp; ++comp)
				{
					oss.str("");
					oss << prefix << "_COLUMN_INLET_COMP_" << std::setfill('0') << std::setw(3) << std::setprecision(0) << comp;
					writer.template vector<double>(oss.str(), _numTimesteps, _curInlet->data() + comp, _nComp);
				}
			}
			else
			{
				oss.str("");
				oss << prefix << "_COLUMN_INLET";
				writer.template matrix<double>(oss.str(), _numTimesteps, _nComp, _curInlet->data(), 1);
			}
		}

		if (_curCfg->storeColumn)
		{
			oss.str("");
			oss << prefix << "_COLUMN";
			_columnLayout[0] = _numTimesteps;
			writer.template tensor<double>(oss.str(), _columnLayout.size(), _columnLayout.data(), _curBulk->data());
		}

		if (_curCfg->storeParticle)
		{
			oss.str("");
			oss << prefix << "_PARTICLE";
			_particleLayout[0] = _numTimesteps;
			writer.template tensor<double>(oss.str(), _particleLayout.size(), _particleLayout.data(), _curParticle->data());
		}

		if (_curCfg->storeFlux)
		{
			oss.str("");
			oss << prefix << "_FLUX";
			_fluxLayout[0] = _numTimesteps;
			writer.template tensor<double>(oss.str(), _fluxLayout.size(), _fluxLayout.data(), _curFlux->data());
		}
	}

	StorageConfig _cfgSolution;
	StorageConfig _cfgSolutionDot;
	StorageConfig _cfgSensitivity;
	StorageConfig _cfgSensitivityDot;
	bool _storeTime;
	bool _splitComponents;

	StorageConfig const* _curCfg;
	std::vector<double>* _curOutlet;
	std::vector<double>* _curInlet;
	std::vector<double>* _curBulk;
	std::vector<double>* _curParticle;
	std::vector<double>* _curFlux;

	std::vector<double> _time;
	std::vector<double> _outlet;
	std::vector<double> _inlet;
	std::vector<double> _column;
	std::vector<double> _particle;
	std::vector<double> _flux;

	std::vector<double> _outletDot;
	std::vector<double> _inletDot;
	std::vector<double> _columnDot;
	std::vector<double> _particleDot;
	std::vector<double> _fluxDot;

	std::vector<std::vector<double>*> _sensOutlet;
	std::vector<std::vector<double>*> _sensInlet;
	std::vector<std::vector<double>*> _sensColumn;
	std::vector<std::vector<double>*> _sensParticle;
	std::vector<std::vector<double>*> _sensFlux;

	std::vector<std::vector<double>*> _sensOutletDot;
	std::vector<std::vector<double>*> _sensInletDot;
	std::vector<std::vector<double>*> _sensColumnDot;
	std::vector<std::vector<double>*> _sensParticleDot;
	std::vector<std::vector<double>*> _sensFluxDot;
	
	std::vector<std::size_t> _columnLayout;
	std::vector<std::size_t> _particleLayout;
	std::vector<std::size_t> _fluxLayout;

	unsigned int _nComp;
	unsigned int _numTimesteps;
	unsigned int _numSens;
	UnitOpIdx _unitOp;

	bool _needsReAlloc;
};


/**
 * @brief Stores pieces of the solution of the whole model system in recorders of single unit operations
 * @details Maintains a collection of InternalStorageUnitOpRecorder objects that store individual unit operations.
 *          The individual unit operation recorders are owned by this object and destroyed upon its own
 *          destruction.
 */
class InternalStorageSystemRecorder : public ISolutionRecorder
{
public:

	InternalStorageSystemRecorder() : _numTimesteps(0), _numSens(0), _storeTime(true)
	{
	}

	virtual ~InternalStorageSystemRecorder() CADET_NOEXCEPT
	{
		deleteRecorders();
	}

	virtual void clear()
	{
		_time.clear();

		for (InternalStorageUnitOpRecorder* rec : _recorders)
			rec->clear();
	}

	virtual void prepare(unsigned int numDofs, unsigned int numSens, unsigned int numTimesteps)
	{
		_numSens = numSens;
		_time.reserve(numTimesteps);

		for (InternalStorageUnitOpRecorder* rec : _recorders)
			rec->prepare(numDofs, numSens, numTimesteps);
	}

	virtual void notifyIntegrationStart(unsigned int numDofs, unsigned int numSens, unsigned int numTimesteps)
	{
		_numSens = numSens;
		_time.clear();
		_time.reserve(numTimesteps);

		for (InternalStorageUnitOpRecorder* rec : _recorders)
			rec->notifyIntegrationStart(numDofs, numSens, numTimesteps);
	}

	virtual void unitOperationStructure(UnitOpIdx idx, const IModel& model, const ISolutionExporter& exporter)
	{
		for (InternalStorageUnitOpRecorder* rec : _recorders)
			rec->unitOperationStructure(idx, model, exporter);

		// Reset for counting actual number of time steps
		_numTimesteps = 0;
	}

	virtual void beginTimestep(double t)
	{
		++_numTimesteps;
		if (_storeTime)
			_time.push_back(t);

		for (InternalStorageUnitOpRecorder* rec : _recorders)
			rec->beginTimestep(t);
	}

	virtual void beginUnitOperation(cadet::UnitOpIdx idx, const cadet::IModel& model, const cadet::ISolutionExporter& exporter)
	{
		for (InternalStorageUnitOpRecorder* rec : _recorders)
			rec->beginUnitOperation(idx, model, exporter);
	}

	virtual void endUnitOperation()
	{
		for (InternalStorageUnitOpRecorder* rec : _recorders)
			rec->endUnitOperation();
	}

	virtual void endTimestep()
	{
		for (InternalStorageUnitOpRecorder* rec : _recorders)
			rec->endTimestep();
	}

	virtual void beginSolution()
	{
		for (InternalStorageUnitOpRecorder* rec : _recorders)
			rec->beginSolution();
	}

	virtual void endSolution()
	{
		for (InternalStorageUnitOpRecorder* rec : _recorders)
			rec->endSolution();
	}

	virtual void beginSolutionDerivative()
	{
		for (InternalStorageUnitOpRecorder* rec : _recorders)
			rec->beginSolutionDerivative();
	}

	virtual void endSolutionDerivative()
	{
		for (InternalStorageUnitOpRecorder* rec : _recorders)
			rec->endSolutionDerivative();
	}

	virtual void beginSensitivity(const cadet::ParameterId& pId, unsigned int sensIdx)
	{
		for (InternalStorageUnitOpRecorder* rec : _recorders)
			rec->beginSensitivity(pId, sensIdx);
	}

	virtual void endSensitivity(const cadet::ParameterId& pId, unsigned int sensIdx)
	{
		for (InternalStorageUnitOpRecorder* rec : _recorders)
			rec->endSensitivity(pId, sensIdx);
	}

	virtual void beginSensitivityDerivative(const cadet::ParameterId& pId, unsigned int sensIdx)
	{
		for (InternalStorageUnitOpRecorder* rec : _recorders)
			rec->beginSensitivityDerivative(pId, sensIdx);
	}

	virtual void endSensitivityDerivative(const cadet::ParameterId& pId, unsigned int sensIdx)
	{
		for (InternalStorageUnitOpRecorder* rec : _recorders)
			rec->endSensitivityDerivative(pId, sensIdx);
	}
	
	template <typename Writer_t>
	void writeSolution(Writer_t& writer)
	{
		std::ostringstream oss;

		if (_storeTime)
			writer.template vector<double>("SOLUTION_TIMES", _time.size(), _time.data());

		for (InternalStorageUnitOpRecorder* rec : _recorders)
		{
			oss.str("");
			oss << "unit_" << std::setfill('0') << std::setw(3) << std::setprecision(0) << static_cast<int>(rec->unitOperation());

			writer.pushGroup(oss.str());
			rec->writeSolution(writer);
			writer.popGroup();
		}
	}

	template <typename Writer_t>
	void writeSensitivity(Writer_t& writer)
	{
		std::ostringstream oss;

		for (unsigned int param = 0; param < _numSens; ++param)
		{
			oss.str("");
			oss << "param_" << std::setfill('0') << std::setw(3) << std::setprecision(0) << param;
			writer.pushGroup(oss.str());

			for (InternalStorageUnitOpRecorder* rec : _recorders)
			{
				oss.str("");
				oss << "unit_" << std::setfill('0') << std::setw(3) << std::setprecision(0) << static_cast<int>(rec->unitOperation());

				writer.pushGroup(oss.str());
				rec->writeSensitivity(writer, param);
				writer.popGroup();
			}

			writer.popGroup();
		}
	}

	inline bool storeTime() const CADET_NOEXCEPT { return _storeTime; }
	inline void storeTime(bool st) CADET_NOEXCEPT { _storeTime = st; }

	inline unsigned int numDataPoints() const CADET_NOEXCEPT { return _numTimesteps; }

	inline void addRecorder(InternalStorageUnitOpRecorder* rec)
	{
		_recorders.push_back(rec);
	}

	inline unsigned int numRecorders() const CADET_NOEXCEPT { return _recorders.size(); }
	inline InternalStorageUnitOpRecorder* recorder(unsigned int idx) CADET_NOEXCEPT { return _recorders[idx]; }
	inline InternalStorageUnitOpRecorder* const recorder(unsigned int idx) const CADET_NOEXCEPT { return _recorders[idx]; }

	inline InternalStorageUnitOpRecorder* unitOperation(UnitOpIdx idx) CADET_NOEXCEPT
	{
		for (InternalStorageUnitOpRecorder* rec : _recorders)
		{
			if (rec->unitOperation() == idx)
				return rec;
		}
		return nullptr;
	}
	inline InternalStorageUnitOpRecorder const* unitOperation(UnitOpIdx idx) const CADET_NOEXCEPT
	{
		for (InternalStorageUnitOpRecorder const* rec : _recorders)
		{
			if (rec->unitOperation() == idx)
				return rec;
		}
		return nullptr;
	}

	inline void deleteRecorders()
	{
		for (InternalStorageUnitOpRecorder* rec : _recorders)
			delete rec;
		_recorders.clear();
	}

protected:

	std::vector<InternalStorageUnitOpRecorder*> _recorders;
	unsigned int _numTimesteps;
	unsigned int _numSens;
	std::vector<double> _time;
	bool _storeTime;
};


} // namespace cadet

#endif  // LIBCADET_SOLUTIONRECORDER_IMPL_HPP_
