#include "EventLoop/StatusCode.h"
#include "EventLoop/Worker.h"
#include "xAODRootAccess/TStore.h"

#include "SUSYTools/SUSYObjDef_xAOD.h"
#include "xAODParticleEvent/ParticleContainer.h"
#include "xAODJet/JetAuxContainer.h"

#include "RJigsawTools/RegionVarCalculator_zl.h"
#include "RJigsawTools/strongErrorCheck.h"

#include <xAODAnaHelpers/HelperFunctions.h>


// this is needed to distribute the algorithm to the workers
ClassImp(RegionVarCalculator_zl)

EL::StatusCode RegionVarCalculator_zl::doInitialize(EL::Worker * worker) {
  if(m_worker != nullptr){
    std::cout << "You have called " << __PRETTY_FUNCTION__ << " more than once.  Exiting." << std::endl;
    return EL::StatusCode::FAILURE;
  }
  m_worker = worker;

  return EL::StatusCode::SUCCESS;
}

EL::StatusCode RegionVarCalculator_zl::doCalculate(std::map<std::string, double              >& RegionVars,
						     std::map<std::string, std::vector<double> >& VecRegionVars){
  xAOD::TStore * store = m_worker->xaodStore();//grab the store from the worker
  xAOD::TEvent* event = m_worker->xaodEvent();

  const xAOD::EventInfo* eventInfo = nullptr;
  STRONG_CHECK(event->retrieve( eventInfo, "EventInfo"));

  std::string const & regionName = eventInfo->auxdecor< std::string >("regionName");

  if      ( regionName.empty() ) {return EL::StatusCode::SUCCESS;}
  // If it hasn't been selected in any of the regions from any of the select algs, don't bother calculating anything...
  else if ( regionName == "SR" )  {return EL::StatusCode(doAllCalculations (RegionVars, VecRegionVars) == EL::StatusCode::SUCCESS &&
							 doSRCalculations  (RegionVars, VecRegionVars) == EL::StatusCode::SUCCESS);}

  else if ( regionName == "CR1L") {return EL::StatusCode(doAllCalculations (RegionVars, VecRegionVars) == EL::StatusCode::SUCCESS &&
							 doCR1LCalculations(RegionVars, VecRegionVars) == EL::StatusCode::SUCCESS);}

  else if ( regionName == "CR2L") {return EL::StatusCode(doAllCalculations (RegionVars, VecRegionVars) == EL::StatusCode::SUCCESS &&
							 doCR2LCalculations(RegionVars, VecRegionVars) == EL::StatusCode::SUCCESS);}


  return EL::StatusCode::SUCCESS;
}

EL::StatusCode RegionVarCalculator_zl::doAllCalculations(std::map<std::string, double>& RegionVars,
							   std::map<std::string, std::vector<double> > & VecRegionVars)
{/*todo*/
  xAOD::TStore * store = m_worker->xaodStore();
  xAOD::TEvent * event = m_worker->xaodEvent();

  //store->print();

  // Get relevant info from the EventInfo object /////////////////////
  //

  const xAOD::EventInfo* eventInfo = nullptr;
  STRONG_CHECK(event->retrieve( eventInfo, "EventInfo"));

  RegionVars["runNumber"]   = eventInfo->runNumber();
  RegionVars["lumiBlock"]   = eventInfo->lumiBlock();
  RegionVars["bcid"]        = eventInfo->bcid();
  RegionVars["eventNumber"] = eventInfo->eventNumber();
  RegionVars["mcChannelNumber"] = eventInfo->mcChannelNumber();
  RegionVars["actualInteractionsPerCrossing"] = eventInfo->actualInteractionsPerCrossing();
  RegionVars["averageInteractionsPerCrossing"] = eventInfo->averageInteractionsPerCrossing();

  RegionVars["mcEventWeight"] = eventInfo->auxdecor< float >("mcEventWeight");
  RegionVars["pileupWeight"]  = eventInfo->auxdecor< float >("PileupWeight");

  //
  /////////////////////////////////////////////////////////////////////

  // Get relevant info from the vertex container //////////////////////
  //

  const xAOD::VertexContainer* vertices = nullptr;
  STRONG_CHECK(event->retrieve( vertices, "PrimaryVertices"));
  RegionVars["NPV"] = HelperFunctions::countPrimaryVertices(vertices, 2);

  //
  /////////////////////////////////////////////////////////////////////

  xAOD::MissingETContainer * metcont = nullptr;
  STRONG_CHECK(store->retrieve(metcont, "STCalibMET"));

  //  std::cout << "MET : " << (*metcont)["Final"]->met() << std::endl;
  RegionVars     ["met"]   = (*metcont)["Final"]->met();

  // xAOD::JetContainer* jets_nominal(nullptr);
  // STRONG_CHECK(store->retrieve(jets_nominal, "STCalibAntiKt4EMTopoJets"));

  xAOD::ParticleContainer* jets_nominal(nullptr);
  STRONG_CHECK(store->retrieve(jets_nominal, "selectedJets"));

  //  const std::vector<xAOD::IParticle*> & jetStdVec = jetcont->stdcont();
  std::vector<double> jetPtVec;
  std::vector<double> jetEtaVec;
  std::vector<double> jetPhiVec;
  std::vector<double> jetEVec;

  for( const auto& jet : *jets_nominal) {
    jetPtVec.push_back( jet->pt());
    jetEtaVec.push_back( jet->p4().Eta() );
    jetPhiVec.push_back( jet->p4().Phi() );
    jetEVec.push_back( jet->p4().E() );
  }

  VecRegionVars[ "jetPt" ]  = jetPtVec;
  VecRegionVars[ "jetEta" ] = jetEtaVec;
  VecRegionVars[ "jetPhi" ] = jetPhiVec;
  VecRegionVars[ "jetE" ]   = jetEVec;

  xAOD::ParticleContainer* leptons_nominal(nullptr);
  STRONG_CHECK(store->retrieve(leptons_nominal, "selectedLeptons"));

  std::vector<double> lepPtVec;
  std::vector<double> lepEtaVec;
  std::vector<double> lepPhiVec;
  std::vector<double> lepEVec;

  for( const auto& lep : *leptons_nominal) {
    lepPtVec.push_back( lep->pt());
    lepEtaVec.push_back( lep->p4().Eta() );
    lepPhiVec.push_back( lep->p4().Phi() );
    lepEVec.push_back( lep->p4().E() );
  }

  VecRegionVars[ "lepPt" ]  = lepPtVec;
  VecRegionVars[ "lepEta" ] = lepEtaVec;
  VecRegionVars[ "lepPhi" ] = lepPhiVec;
  VecRegionVars[ "lepE" ]   = lepEVec;

  return EL::StatusCode::SUCCESS;
}


EL::StatusCode RegionVarCalculator_zl::doSRCalculations(std::map<std::string, double>& RegionVars,
							  std::map<std::string, std::vector<double> > & VecRegionVars)
{/*todo*/return EL::StatusCode::SUCCESS;}


EL::StatusCode RegionVarCalculator_zl::doCR1LCalculations(std::map<std::string, double>& RegionVars,
							    std::map<std::string, std::vector<double> > & VecRegionVars)
{/*todo*/return EL::StatusCode::SUCCESS;}


EL::StatusCode RegionVarCalculator_zl::doCR2LCalculations(std::map<std::string, double>& RegionVars,
							    std::map<std::string, std::vector<double> > & VecRegionVars)
{/*todo*/return EL::StatusCode::SUCCESS;}
