#include <EventLoop/Job.h>
#include <EventLoop/StatusCode.h>
#include <EventLoop/Worker.h>
#include <FactoryTools/SelectZeroLeptonEvents.h>

#include <AsgTools/MsgStream.h>
#include <AsgTools/MsgStreamMacros.h>

#include "xAODRootAccess/Init.h"
#include "xAODRootAccess/TEvent.h"
#include "xAODRootAccess/TStore.h"

#include "SUSYTools/SUSYObjDef_xAOD.h"

#include <FactoryTools/strongErrorCheck.h>

#include "xAODParticleEvent/ParticleContainer.h"
#include "xAODParticleEvent/ParticleAuxContainer.h"

#include <boost/algorithm/string.hpp>


// this is needed to distribute the algorithm to the workers
ClassImp(SelectZeroLeptonEvents)



SelectZeroLeptonEvents :: SelectZeroLeptonEvents ()
{
  // Here you put any code for the base initialization of variables,
  // e.g. initialize all pointers to 0.  Note that you should only put
  // the most basic initialization here, since this method will be
  // called on both the submission and the worker node.  Most of your
  // initialization code will go into histInitialize() and
  // initialize().
}



EL::StatusCode SelectZeroLeptonEvents :: setupJob (EL::Job& job)
{
  // Here you put code that sets up the job on the submission object
  // so that it is ready to work with your algorithm, e.g. you can
  // request the D3PDReader service or add output files.  Any code you
  // put here could instead also go into the submission script.  The
  // sole advantage of putting it here is that it gets automatically
  // activated/deactivated when you add/remove the algorithm from your
  // job, which may or may not be of value to you.
  return EL::StatusCode::SUCCESS;
}



EL::StatusCode SelectZeroLeptonEvents :: histInitialize ()
{
  // Here you do everything that needs to be done at the very
  // beginning on each worker node, e.g. create histograms and output
  // trees.  This method gets called before any input files are
  // connected.
  return EL::StatusCode::SUCCESS;
}



EL::StatusCode SelectZeroLeptonEvents :: fileExecute ()
{
  // Here you do everything that needs to be done exactly once for every
  // single file, e.g. collect a list of all lumi-blocks processed
  return EL::StatusCode::SUCCESS;
}



EL::StatusCode SelectZeroLeptonEvents :: changeInput (bool firstFile)
{
  // Here you do everything you need to do when we change input files,
  // e.g. resetting branch addresses on trees.  If you are using
  // D3PDReader or a similar service this method is not needed.
  return EL::StatusCode::SUCCESS;
}



EL::StatusCode SelectZeroLeptonEvents :: initialize ()
{
  // Here you do everything that you need to do after the first input
  // file has been connected and before the first event is processed,
  // e.g. create additional histograms based on which variables are
  // available in the input files.  You can also create all of your
  // histograms and trees in here, but be aware that this method
  // doesn't get called if no events are processed.  So any objects
  // you create here won't be available in the output if you have no
  // input events.
  return EL::StatusCode::SUCCESS;
}



EL::StatusCode SelectZeroLeptonEvents :: execute ()
{
  // Here you do everything that needs to be done on every single
  // events, e.g. read input variables, apply cuts, and fill
  // histograms and trees.  This is where most of your actual analysis
  // code will go.

  //todo add some preselection here!

  xAOD::TStore * store = wk()->xaodStore();
  xAOD::TEvent* event = wk()->xaodEvent();

  const xAOD::EventInfo* eventInfo = 0;
  STRONG_CHECK(event->retrieve( eventInfo, "EventInfo"));

  // If the event didn't pass the preselection alg, don't bother doing anything with it...
  std::string preselectedRegionName =  eventInfo->auxdecor< std::string >("regionName");
  ATH_MSG_DEBUG("EventNumber: " << eventInfo->eventNumber()  );
  ATH_MSG_DEBUG("Preselected?: " << preselectedRegionName  );

  if( preselectedRegionName == "" ) return EL::StatusCode::SUCCESS;

  std::pair<xAOD::IParticleContainer* , xAOD::ParticleAuxContainer*> selectedLeptons( new xAOD::IParticleContainer(SG::VIEW_ELEMENTS) , nullptr);
  selectedLeptons.first->setStore(selectedLeptons.second);

  std::pair<xAOD::IParticleContainer* , xAOD::ParticleAuxContainer*> selectedBaselineLeptons( new xAOD::IParticleContainer(SG::VIEW_ELEMENTS) , nullptr);
  selectedBaselineLeptons.first->setStore(selectedBaselineLeptons.second);

  std::pair<xAOD::IParticleContainer* , xAOD::ParticleAuxContainer*> selectedPhotons( new xAOD::IParticleContainer(SG::VIEW_ELEMENTS) , nullptr);
  selectedPhotons.first->setStore(selectedPhotons.second);

  std::pair<xAOD::IParticleContainer* , xAOD::ParticleAuxContainer*> selectedJets   ( new xAOD::IParticleContainer(SG::VIEW_ELEMENTS) , nullptr);
  selectedJets.first->setStore(selectedJets.second);
  std::pair<xAOD::IParticleContainer* , xAOD::ParticleAuxContainer *> selectedRJigsawParticles(new xAOD::IParticleContainer(SG::VIEW_ELEMENTS), nullptr);
  selectedRJigsawParticles.first->setStore(selectedRJigsawParticles.second);

  STRONG_CHECK( store->record( selectedLeptons.first  , "selectedLeptons"    ) );//todo configurable if needed
  STRONG_CHECK( store->record( selectedLeptons.second , "selectedLeptonsAux."    ) );//todo configurable if needed

  STRONG_CHECK( store->record( selectedBaselineLeptons.first  , "selectedBaselineLeptons"    ) );//todo configurable if needed
  STRONG_CHECK( store->record( selectedBaselineLeptons.second , "selectedBaselineLeptonsAux."    ) );//todo configurable if needed

  STRONG_CHECK( store->record( selectedPhotons.first  , "selectedPhotons"    ) );//todo configurable if needed
  STRONG_CHECK( store->record( selectedPhotons.second , "selectedPhotonsAux."    ) );//todo configurable if needed

  STRONG_CHECK( store->record( selectedJets.first     , "selectedJets"    ) );//todo configurable if needed
  STRONG_CHECK( store->record( selectedJets.second    , "selectedJetsAux."    ) );//todo configurable if needed

  STRONG_CHECK( store->record( selectedRJigsawParticles.first  , "myparticles"    ) );//todo configurable if needed
  STRONG_CHECK( store->record( selectedRJigsawParticles.second , "myparticlesaux."    ) );//todo configurable if needed


  xAOD::JetContainer* jets_nominal(nullptr);
  STRONG_CHECK(store->retrieve(jets_nominal, "STCalibAntiKt4EMTopoJets"));

  xAOD::MuonContainer* muons_nominal(nullptr);
  STRONG_CHECK(store->retrieve(muons_nominal, "STCalibMuons"));

  xAOD::ElectronContainer* electrons_nominal(nullptr);
  STRONG_CHECK(store->retrieve(electrons_nominal, "STCalibElectrons"));

  xAOD::PhotonContainer* photons_nominal(nullptr);
  STRONG_CHECK(store->retrieve(photons_nominal, "STCalibPhotons"));



  for (const auto& jet : *jets_nominal) {
    if ((int)jet->auxdata<char>("baseline") == 0) continue;
    if ((int)jet->auxdata<char>("passOR") != 1) continue;
    if ((int)jet->auxdata<char>("signal") != 1) continue;
    // If I've gotten this far, I have a signal, isolated, beautiful jet
    ATH_MSG_VERBOSE( "jet pt : " << jet->pt() );

    selectedJets.first->push_back(jet  );
  }

  for (const auto& mu : *muons_nominal) {
    if ((int)mu->auxdata<char>("baseline") == 0) continue;
    if ((int)mu->auxdata<char>("passOR") != 1) continue;

    selectedBaselineLeptons.first->push_back( mu );

    if ((int)mu->auxdata<char>("signal") != 1) continue;
    // If I've gotten this far, I have a signal, isolated, beautiful muon
    ATH_MSG_VERBOSE( "mu pt : " << mu->pt() );

    selectedLeptons.first->push_back( mu );
  }

  for (const auto& el : *electrons_nominal) {
    if ((int)el->auxdata<char>("baseline") == 0) continue;
    if ((int)el->auxdata<char>("passOR") != 1) continue;

    selectedBaselineLeptons.first->push_back( el );

    if ((int)el->auxdata<char>("signal") != 1) continue;
    // If I've gotten this far, I have a signal, isolated, beautiful el
    ATH_MSG_VERBOSE( "el pt : " << el->pt() );

    selectedLeptons.first->push_back( el );
  }


  for (const auto& ph : *photons_nominal) {
    if ((int)ph->auxdata<char>("baseline") == 0) continue;
    if ((int)ph->auxdata<char>("passOR") != 1) continue;
    if ((int)ph->auxdata<char>("signal") != 1) continue;
    // If I've gotten this far, I have a signal, isolated, beautiful el
    ATH_MSG_VERBOSE( "ph pt : " << ph->pt() );

    selectedPhotons.first->push_back( ph );
  }

  int const nLeptons = selectedLeptons.first->size();
  int const nBaselineLeptons = selectedBaselineLeptons.first->size();
  int const nPhotons = selectedPhotons.first->size();

  ATH_MSG_DEBUG("Number of Selected Leptons: " << nLeptons  );
  ATH_MSG_DEBUG("Number of Selected Baseline Leptons: " << nBaselineLeptons  );
  ATH_MSG_DEBUG("Number of Selected Photons: " << nPhotons  );

  bool passTM = false;
  for (auto lepton: *selectedLeptons.first){
    if(lepton->auxdecor< int >( "passTM" ) ) passTM = true;
  }


  auto trigORFromString = [](std::vector< std::string > passTrigs, std::string trigString){
    boost::replace_all(trigString, "_OR_", ":");
    std::vector<std::string> trigVect;
    boost::split(trigVect,trigString,boost::is_any_of(":") );
    bool trigDecision = 0;
    for(auto iTrig : trigVect){
      trigDecision |= std::find(passTrigs.begin(), passTrigs.end(), iTrig ) != passTrigs.end();
    }
    return trigDecision;
  };



  std::vector< std::string > const & passTrigs = eventInfo->auxdecor<  std::vector< std::string >  >("passTriggers");


  std::map<std::string,int> passedTriggers;
  passedTriggers["HLT_xe100"] = std::find(passTrigs.begin(), passTrigs.end(), "HLT_xe100") != passTrigs.end();
  passedTriggers["HLT_xe100_mht_L1XE50"] = std::find(passTrigs.begin(), passTrigs.end(), "HLT_xe100_mht_L1XE50") != passTrigs.end();

  // passedTriggers["HLT_e24_lhmedium_L1EM18VH"] = std::find(passTrigs.begin(), passTrigs.end(), "HLT_e24_lhmedium_L1EM18VH") != passTrigs.end();
  // passedTriggers["HLT_e26_lhtight_nod0_ivarloose"] = std::find(passTrigs.begin(), passTrigs.end(), "HLT_e26_lhtight_nod0_ivarloose") != passTrigs.end();
  // passedTriggers["HLT_mu20_iloose_L1MU15"] = std::find(passTrigs.begin(), passTrigs.end(), "HLT_mu20_iloose_L1MU15") != passTrigs.end();
  // passedTriggers["HLT_mu26_ivarmedium"] = std::find(passTrigs.begin(), passTrigs.end(), "HLT_mu26_ivarmedium") != passTrigs.end();

  passedTriggers["HLT_g140_loose"] = std::find(passTrigs.begin(), passTrigs.end(), "HLT_g140_loose") != passTrigs.end();


  if(eventInfo->auxdecor<float>("year")==2015){
    passedTriggers["MET"] = passedTriggers["HLT_xe100_mht_L1XE50"];
    passedTriggers["Electron"] = trigORFromString(passTrigs,eventInfo->auxdecor<std::string>("elTrig2015") );
    passedTriggers["Muon"] = trigORFromString(passTrigs,eventInfo->auxdecor<std::string>("muTrig2015") );
    passedTriggers["Photon"] = passedTriggers["HLT_g140_loose"];
  } else {
    passedTriggers["MET"] = passedTriggers["HLT_xe100_mht_L1XE50"];
    passedTriggers["Electron"] = trigORFromString(passTrigs,eventInfo->auxdecor<std::string>("elTrig2016") );
    passedTriggers["Muon"] = trigORFromString(passTrigs,eventInfo->auxdecor<std::string>("muTrig2016") );
    passedTriggers["Photon"] = passedTriggers["HLT_g140_loose"];
  }


  auto  getRegionName = [](int nLeptons, int nBaselineLeptons, int nPhotons, std::map<std::string,int>  passedTriggers, bool passTM){ 
            std::string regionName = "";
					  if( nBaselineLeptons == 0 && passedTriggers["MET"] ) {regionName = "SR";}
					  if( nLeptons == 1 && (passedTriggers["Electron"]||passedTriggers["Muon"]) && passTM){regionName = "CR1L";}
            if( nLeptons == 2 && (passedTriggers["Electron"]||passedTriggers["Muon"]) && passTM){regionName = "CR2L";}
            if( nPhotons > 0  && passedTriggers["Photon"] ){regionName = "CRY";}
					  return regionName;};//todo >2 leptons in the SR? pretty rare though


  ATH_MSG_DEBUG("Event falls in region: " << getRegionName( nLeptons, nBaselineLeptons, nPhotons, passedTriggers, passTM )  );

  ATH_MSG_DEBUG("Writing particle container for calculator to store");

  eventInfo->auxdecor< std::string >("regionName") = getRegionName( nLeptons, nBaselineLeptons, nPhotons, passedTriggers, passTM ) ;


  // // What happens if we add the jets into the calculation?
  for( const auto& myjet: *selectedJets.first){
    selectedRJigsawParticles.first->push_back( myjet );
  }

  if (eventInfo->auxdecor< std::string >("regionName") == "CR1L"){
    for( const auto& mylep: *selectedLeptons.first){
      selectedRJigsawParticles.first->push_back( mylep );
    }
  }


  ATH_MSG_DEBUG("Writing to eventInfo decoration: " <<  eventInfo->auxdecor< std::string >("regionName")   );




  return EL::StatusCode::SUCCESS;
}



EL::StatusCode SelectZeroLeptonEvents :: postExecute ()
{
  // Here you do everything that needs to be done after the main event
  // processing.  This is typically very rare, particularly in user
  // code.  It is mainly used in implementing the NTupleSvc.
  return EL::StatusCode::SUCCESS;
}



EL::StatusCode SelectZeroLeptonEvents :: finalize ()
{
  // This method is the mirror image of initialize(), meaning it gets
  // called after the last event has been processed on the worker node
  // and allows you to finish up any objects you created in
  // initialize() before they are written to disk.  This is actually
  // fairly rare, since this happens separately for each worker node.
  // Most of the time you want to do your post-processing on the
  // submission node after all your histogram outputs have been
  // merged.  This is different from histFinalize() in that it only
  // gets called on worker nodes that processed input events.
  return EL::StatusCode::SUCCESS;
}



EL::StatusCode SelectZeroLeptonEvents :: histFinalize ()
{
  // This method is the mirror image of histInitialize(), meaning it
  // gets called after the last event has been processed on the worker
  // node and allows you to finish up any objects you created in
  // histInitialize() before they are written to disk.  This is
  // actually fairly rare, since this happens separately for each
  // worker node.  Most of the time you want to do your
  // post-processing on the submission node after all your histogram
  // outputs have been merged.  This is different from finalize() in
  // that it gets called on all worker nodes regardless of whether
  // they processed input events.
  return EL::StatusCode::SUCCESS;
}
