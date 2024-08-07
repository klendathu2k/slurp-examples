#ifndef FUN4ALL_YEAR1_C
#define FUN4ALL_YEAR1_C

#include <QA.C>

#include <caloreco/CaloTowerBuilder.h>
#include <caloreco/CaloTowerCalib.h>
#include <caloreco/CaloTowerStatus.h>
#include <caloreco/CaloWaveformProcessing.h>
#include <caloreco/DeadHotMapLoader.h>
#include <caloreco/RawClusterBuilderTemplate.h>
#include <caloreco/RawClusterDeadHotMask.h>
#include <caloreco/RawClusterPositionCorrection.h>
#include <caloreco/TowerInfoDeadHotMask.h>

#include <mbd/MbdReco.h>

#include <zdcinfo/ZdcReco.h>

#include <globalvertex/GlobalVertexReco.h>

#include <ffamodules/CDBInterface.h>
#include <ffamodules/FlagHandler.h>
#include <ffamodules/HeadReco.h>

#include <fun4allraw/Fun4AllPrdfInputManager.h>

#include <fun4all/Fun4AllDstInputManager.h>
#include <fun4all/Fun4AllDstOutputManager.h>
#include <fun4all/Fun4AllInputManager.h>
#include <fun4all/Fun4AllRunNodeInputManager.h>
#include <fun4all/Fun4AllServer.h>
#include <fun4all/Fun4AllUtils.h>
#include <fun4all/SubsysReco.h>

#include <phool/recoConsts.h>

#include <centrality/CentralityReco.h>
#include <calotrigger/MinimumBiasClassifier.h>

#include <calovalid/CaloValid.h>
#include <globalqa/GlobalQA.h>

R__LOAD_LIBRARY(libfun4all.so)
R__LOAD_LIBRARY(libfun4allraw.so)
R__LOAD_LIBRARY(libcalo_reco.so)
R__LOAD_LIBRARY(libcalotrigger.so)
R__LOAD_LIBRARY(libcentrality.so)
R__LOAD_LIBRARY(libffamodules.so)
R__LOAD_LIBRARY(libmbd.so)
R__LOAD_LIBRARY(libzdcinfo.so)
R__LOAD_LIBRARY(libglobalvertex.so)
R__LOAD_LIBRARY(libcalovalid.so)
R__LOAD_LIBRARY(libglobalQA.so)

void Fun4All_Year2(int nEvents=0,
		   const std::string &fname = "/sphenix/lustre01/sphnxpro/commissioning/slurp/calobeam/run_00040700_00040800/DST_TRIGGERED_RAW_beam_new_2023p015-00040797-0001.root",
		   const std::string& outfile="",
		   const std::string& outfile_hist="",
		   const std::string& dbtag="-nope-"
  )
{
  bool addZeroSupCaloNodes = 0;
  // v1 uncomment:
  // CaloTowerDefs::BuilderType buildertype = CaloTowerDefs::kPRDFTowerv1;
  // v2 uncomment:
  // CaloTowerDefs::BuilderType buildertype = CaloTowerDefs::kWaveformTowerv2;
   CaloTowerDefs::BuilderType buildertype = CaloTowerDefs::kPRDFTowerv4;
  // v3 uncomment:
  // CaloTowerDefs::BuilderType buildertype = CaloTowerDefs::kPRDFWaveform;

  Fun4AllServer *se = Fun4AllServer::instance();
  se->Verbosity(0);

  recoConsts *rc = recoConsts::instance();

  pair<int, int> runseg = Fun4AllUtils::GetRunSegment(fname);
  int runnumber = runseg.first;
  int segment = runseg.second;
//  char outfile[100];
//  char outfile_hist[100];
//  sprintf(outfile, "DST_CALOR-%08d-%04d.root", runnumber, segment);
//  sprintf(outfile_hist, "HIST_CALOR-%08d-%04d.root", runnumber, segment);
  string fulloutfile = string("./") + outfile;
  string fulloutfile_hist = string("./") + outfile_hist;
  //===============
  // conditions DB flags
  //===============
  // ENABLE::CDB = true;
  // global tag
  rc->set_StringFlag("CDB_GLOBALTAG", dbtag );
  // // 64 bit timestamp
  rc->set_uint64Flag("TIMESTAMP", runnumber);
  CDBInterface::instance()->Verbosity(1);

  FlagHandler *flag = new FlagHandler();
  se->registerSubsystem(flag);

  // MBD/BBC Reconstruction
  MbdReco *mbdreco = new MbdReco();
  se->registerSubsystem(mbdreco);

  CaloTowerBuilder *caZDC = new CaloTowerBuilder("ZDCBUILDER");
  caZDC->set_detector_type(CaloTowerDefs::ZDC);
  caZDC->set_builder_type(buildertype);
  caZDC->set_processing_type(CaloWaveformProcessing::FAST);
  caZDC->set_nsamples(16);
  caZDC->set_offlineflag();
  se->registerSubsystem(caZDC);

  //ZDC Reconstruction--Calib Info
  ZdcReco *zdcreco = new ZdcReco();
  se->registerSubsystem(zdcreco);

  // Official vertex storage
  GlobalVertexReco *gvertex = new GlobalVertexReco();
  se->registerSubsystem(gvertex);

  /////////////////
  // build towers
  CaloTowerBuilder *ctbEMCal = new CaloTowerBuilder("EMCalBUILDER");
  ctbEMCal->set_detector_type(CaloTowerDefs::CEMC);
  ctbEMCal->set_processing_type(CaloWaveformProcessing::TEMPLATE);
  ctbEMCal->set_builder_type(buildertype);
  ctbEMCal->set_offlineflag(true);
  ctbEMCal->set_nsamples(12);
  ctbEMCal->set_bitFlipRecovery(true);
  se->registerSubsystem(ctbEMCal);

  CaloTowerBuilder *ctbIHCal = new CaloTowerBuilder("HCALINBUILDER");
  ctbIHCal->set_detector_type(CaloTowerDefs::HCALIN);
  ctbIHCal->set_processing_type(CaloWaveformProcessing::TEMPLATE);
  ctbIHCal->set_builder_type(buildertype);
  ctbIHCal->set_offlineflag();
  ctbIHCal->set_nsamples(12);
  ctbIHCal->set_bitFlipRecovery(true);
  se->registerSubsystem(ctbIHCal);

  CaloTowerBuilder *ctbOHCal = new CaloTowerBuilder("HCALOUTBUILDER");
  ctbOHCal->set_detector_type(CaloTowerDefs::HCALOUT);
  ctbOHCal->set_processing_type(CaloWaveformProcessing::TEMPLATE);
  ctbOHCal->set_builder_type(buildertype);
  ctbOHCal->set_offlineflag();
  ctbOHCal->set_nsamples(12);
  ctbOHCal->set_bitFlipRecovery(true);
  se->registerSubsystem(ctbOHCal);

  CaloTowerBuilder *caEPD = new CaloTowerBuilder("SEPDBUILDER");
  caEPD->set_detector_type(CaloTowerDefs::SEPD);
  caEPD->set_builder_type(buildertype);
  caEPD->set_processing_type(CaloWaveformProcessing::FAST);
  caEPD->set_nsamples(12);
  caEPD->set_offlineflag();
  se->registerSubsystem(caEPD);


  //////////////////////////////
  // set statuses on raw towers
  
  std::cout << "status setters" << std::endl;
  CaloTowerStatus *statusEMC = new CaloTowerStatus("CEMCSTATUS");
  statusEMC->set_detector_type(CaloTowerDefs::CEMC);
  statusEMC->set_time_cut(1);
  se->registerSubsystem(statusEMC);

  CaloTowerStatus *statusHCalIn = new CaloTowerStatus("HCALINSTATUS");
  statusHCalIn->set_detector_type(CaloTowerDefs::HCALIN);
  statusHCalIn->set_time_cut(2);
  se->registerSubsystem(statusHCalIn);

  CaloTowerStatus *statusHCALOUT = new CaloTowerStatus("HCALOUTSTATUS");
  statusHCALOUT->set_detector_type(CaloTowerDefs::HCALOUT);
  statusHCALOUT->set_time_cut(2);
  se->registerSubsystem(statusHCALOUT);

  ////////////////////
  // Calibrate towers
  std::cout << "Calibrating EMCal" << std::endl;
  CaloTowerCalib *calibEMC = new CaloTowerCalib("CEMCCALIB");
  calibEMC->set_detector_type(CaloTowerDefs::CEMC);
  se->registerSubsystem(calibEMC);

  std::cout << "Calibrating OHcal" << std::endl;
  CaloTowerCalib *calibOHCal = new CaloTowerCalib("HCALOUT");
  calibOHCal->set_detector_type(CaloTowerDefs::HCALOUT);
  se->registerSubsystem(calibOHCal);

  std::cout << "Calibrating IHcal" << std::endl;
  CaloTowerCalib *calibIHCal = new CaloTowerCalib("HCALIN");
  calibIHCal->set_detector_type(CaloTowerDefs::HCALIN);
  se->registerSubsystem(calibIHCal);

  std::cout << "Calibrating ZDC" << std::endl;
  CaloTowerCalib *calibZDC = new CaloTowerCalib("ZDC");
  calibZDC->set_detector_type(CaloTowerDefs::ZDC);
  se->registerSubsystem(calibZDC);

  /////////////////////
  // Geometry 
  std::cout << "Adding Geometry file" << std::endl;
  Fun4AllInputManager *intrue2 = new Fun4AllRunNodeInputManager("DST_GEO");
  std::string geoLocation = CDBInterface::instance()->getUrl("calo_geo");
  intrue2->AddFile(geoLocation);
  se->registerInputManager(intrue2);

  //////////////////
  // Clusters
  
  std::cout << "Building clusters" << std::endl;
  RawClusterBuilderTemplate *ClusterBuilder = new RawClusterBuilderTemplate("EmcRawClusterBuilderTemplate");
  ClusterBuilder->Detector("CEMC");
  ClusterBuilder->set_threshold_energy(0.030);  // for when using basic calibration
  std::string emc_prof = getenv("CALIBRATIONROOT");
  emc_prof += "/EmcProfile/CEMCprof_Thresh30MeV.root";
  ClusterBuilder->LoadProfile(emc_prof);
  ClusterBuilder->set_UseTowerInfo(1);  // to use towerinfo objects rather than old RawTower
  se->registerSubsystem(ClusterBuilder);

  std::cout << "Applying Position Dependent Correction" << std::endl;
  RawClusterPositionCorrection *clusterCorrection = new RawClusterPositionCorrection("CEMC");
  clusterCorrection->set_UseTowerInfo(1);  // to use towerinfo objects rather than old RawTower
  se->registerSubsystem(clusterCorrection);

  ///////////////////////////////////////////
  // Calo node with software zero supression
  if (addZeroSupCaloNodes)
  {
    CaloTowerBuilder *ctbEMCal_SZ = new CaloTowerBuilder("EMCalBUILDER_ZS");
    ctbEMCal_SZ->set_detector_type(CaloTowerDefs::CEMC);
    ctbEMCal_SZ->set_processing_type(CaloWaveformProcessing::TEMPLATE);
    ctbEMCal_SZ->set_nsamples(8);
    ctbEMCal_SZ->set_offlineflag();
    ctbEMCal_SZ->set_outputNodePrefix("TOWERS_SZ_");
    ctbEMCal_SZ->set_softwarezerosuppression(true, 100000000);
    se->registerSubsystem(ctbEMCal_SZ);

    CaloTowerBuilder *ctbIHCal_SZ = new CaloTowerBuilder("HCALINBUILDER_ZS");
    ctbIHCal_SZ->set_detector_type(CaloTowerDefs::HCALIN);
    ctbIHCal_SZ->set_processing_type(CaloWaveformProcessing::TEMPLATE);
    ctbIHCal_SZ->set_offlineflag();
    ctbIHCal_SZ->set_nsamples(8);
    ctbIHCal_SZ->set_outputNodePrefix("TOWERS_SZ_");
    ctbIHCal_SZ->set_softwarezerosuppression(true, 100000000);
    se->registerSubsystem(ctbIHCal_SZ);

    CaloTowerBuilder *ctbOHCal_SZ = new CaloTowerBuilder("HCALOUTBUILDER_SZ");
    ctbOHCal_SZ->set_detector_type(CaloTowerDefs::HCALOUT);
    ctbOHCal_SZ->set_processing_type(CaloWaveformProcessing::TEMPLATE);
    ctbIHCal_SZ->set_nsamples(8);
    ctbOHCal_SZ->set_offlineflag();
    ctbOHCal_SZ->set_outputNodePrefix("TOWERS_SZ_");
    ctbOHCal_SZ->set_softwarezerosuppression(true, 100000000);
    se->registerSubsystem(ctbOHCal_SZ);

    CaloTowerCalib *calibEMC_SZ = new CaloTowerCalib("CEMCCALIB_SZ");
    calibEMC_SZ->set_detector_type(CaloTowerDefs::CEMC);
    calibEMC_SZ->set_inputNodePrefix("TOWERS_SZ_");
    calibEMC_SZ->set_outputNodePrefix("TOWERINFO_SZ_CALIB_");
    se->registerSubsystem(calibEMC_SZ);

    CaloTowerCalib *calibIHCal_SZ = new CaloTowerCalib("IHCALCALIB_SZ");
    calibIHCal_SZ->set_detector_type(CaloTowerDefs::HCALIN);
    calibIHCal_SZ->set_inputNodePrefix("TOWERS_SZ_");
    calibIHCal_SZ->set_outputNodePrefix("TOWERINFO_SZ_CALIB_");
    se->registerSubsystem(calibIHCal_SZ);

    CaloTowerCalib *calibOHCal_SZ = new CaloTowerCalib("OHCALCALIB_SZ");
    calibOHCal_SZ->set_detector_type(CaloTowerDefs::HCALOUT);
    calibOHCal_SZ->set_inputNodePrefix("TOWERS_SZ_");
    calibOHCal_SZ->set_outputNodePrefix("TOWERINFO_SZ_CALIB_");
    se->registerSubsystem(calibOHCal_SZ);
  }

  /////////////////
  // Centrality 
  //MinimumBiasClassifier *minimumbiasclassifier = new MinimumBiasClassifier();
  //se->registerSubsystem(minimumbiasclassifier);
//
  //CentralityReco *centralityreco = new CentralityReco();
  //se->registerSubsystem(centralityreco);

  ///////////////////////////////////
  // Validation 
  CaloValid *ca = new CaloValid("CaloValid");
  ca->set_timing_cut_width(200);  //integers for timing width, > 1 : wider cut around max peak time
  se->registerSubsystem(ca);

  GlobalQA *gqa = new GlobalQA("GlobalQA");
  se->registerSubsystem(gqa);

  Fun4AllInputManager *In = new Fun4AllDstInputManager("in");
  In->AddFile(fname);
  se->registerInputManager(In);

  Fun4AllDstOutputManager *out = new Fun4AllDstOutputManager("DSTOUT", fulloutfile);
  out->StripNode("CEMCPackets");
  out->StripNode("HCALPackets");
  out->StripNode("ZDCPackets");
  out->StripNode("SEPDPackets");
  out->StripNode("MBDPackets");
  se->registerOutputManager(out);

  se->run(nEvents);
  se->End();

  TString qaname = fulloutfile_hist + "_qa.root";
  std::string qaOutputFileName(qaname.Data());
  QAHistManagerDef::saveQARootFile(qaOutputFileName);

  CDBInterface::instance()->Print();  // print used DB files
  se->PrintTimer();
  delete se;
  std::cout << "All done!" << std::endl;
  gSystem->Exit(0);
}

#endif
