///
/// \file   ITSQCTrhesholdTask.cxx
/// \author Barthelemy von Haller
/// \author Piotr Konopka
///

#include <sstream>

#include <TStopwatch.h>
#include "DataFormatsParameters/GRPObject.h"
#include "FairLogger.h"
#include "FairRunAna.h"
#include "FairFileSource.h"
#include "FairRuntimeDb.h"
#include "FairParRootFileIo.h"
#include "FairSystemInfo.h"

#include <TCanvas.h>
#include <TH1.h>
#include <TH2.h>

#include "QualityControl/FileFinish.h"
#include "QualityControl/QcInfoLogger.h"
#include "ITSQCTrhesholdTask/ITSQCTrhesholdTask.h"
//#include "ITSQCTrhesholdTask/ITSQCTrhesholdTaskVariables.h"
#include "DetectorsBase/GeometryManager.h"
#include "ITSBase/GeometryTGeo.h"
#include "ITSMFTReconstruction/DigitPixelReader.h"
#include <algorithm>
//#include "ITSMFTBase/Digit.h"

using o2::itsmft::Digit;

using namespace std;
using namespace o2::itsmft;
using namespace o2::its;

namespace o2
{
namespace quality_control_modules
{
namespace itsqctrhesholdtask
{

ITSQCTrhesholdTask::ITSQCTrhesholdTask() : TaskInterface()
{
  //---------------------------------------------------------------------------AID

  createGlobalHistos();

  for (Int_t iLayer = 0; iLayer < 3; iLayer++) {

    hThresholdSummary_IB[iLayer] = new TH2D(Form("Threshold/Layer%d/Threshold_Vs_Chip_and_Stave", iLayer), Form("ITS Layer%d, Threshold Vs Chip and Stave", iLayer), 9, 0, 9, NStaves[iLayer], 0, NStaves[iLayer]);
    formatStatistics(hThresholdSummary_IB[iLayer], 8, 12);
    formatAxes(hThresholdSummary_IB[iLayer], "Chip Number", "Stave Number", 1, 1.10);
    hThresholdSummary_IB[iLayer]->SetTitle(Form("ITS Layer%d, Threshold Vs Chip and Stave", iLayer));
    addObject(hThresholdSummary_IB[iLayer]);

    // hDeadPixelSummary_IB_IB[iLayer] = new TH2D( Form("DeadPixel/Layer%d/DeadPixel_Vs_Chip_and_Stave",iLayer),Form("ITS Layer%d, DeadPixel Vs Chip and Stave",iLayer),9,0.5,9.5,NStaves[iLayer],0.5,NStaves[iLayer]+0.5);
    hDeadPixelSummary_IB[iLayer] = new TH2D(Form("DeadPixel/Layer%d/DeadPixel_Vs_Chip_and_Stave", iLayer), Form("ITS Layer%d, DeadPixel Vs Chip and Stave", iLayer), 9, 0, 9, NStaves[iLayer], 0, NStaves[iLayer]);
    formatStatistics(hDeadPixelSummary_IB[iLayer], 0, 5000);
    formatAxes(hDeadPixelSummary_IB[iLayer], "Chip Number", "Stave Number", 1, 1.10);
    hDeadPixelSummary_IB[iLayer]->SetTitle(Form("ITS Layer%d, DeadPixel Vs Chip and Stave", iLayer));
    addObject(hDeadPixelSummary_IB[iLayer]);
  }
  for (Int_t iLayer = 3; iLayer < 7; iLayer++) {

    hTresholdSummary_OB_Stave_vs_Hic[iLayer] = new TH2D(Form("Threshold/Layer%d/Threshold_Vs_Stave_and_Hic", iLayer), Form("ITS Layer%d, Threshold Vs Stave and Hic", iLayer), nHicPerStave[iLayer], 0, nHicPerStave[iLayer], NStaves[iLayer], 0, NStaves[iLayer]);
    formatAxes(hTresholdSummary_OB_Stave_vs_Hic[iLayer], "HIC Number", "Stave Number", 1, 1.10);
    hTresholdSummary_OB_Stave_vs_Hic[iLayer]->SetTitle(Form("ITS Layer%d, Threshold  Stave VS HIC", iLayer));
    addObject(hTresholdSummary_OB_Stave_vs_Hic[iLayer]);

    hDeadPixelSummary_OB_Stave_vs_Hic[iLayer] = new TH2D(Form("DeadPixel/Layer%d/DeadPixel_Vs_Stave_and_Hic", iLayer), Form("ITS Layer%d, DeadPixel Vs Stave and Hic", iLayer), nHicPerStave[iLayer], 0, nHicPerStave[iLayer], NStaves[iLayer], 0, NStaves[iLayer]);
    formatAxes(hDeadPixelSummary_OB_Stave_vs_Hic[iLayer], "HIC Number", "Stave Number", 1, 1.10);
    hDeadPixelSummary_OB_Stave_vs_Hic[iLayer]->SetTitle(Form("ITS Layer%d, DeadPixel Stave VS HIC", iLayer));
    addObject(hDeadPixelSummary_OB_Stave_vs_Hic[iLayer]);
  }

  //outTree = new TTree ("StaveQualTest_test","StaveQualTest_test");
  //outTree->Branch("hicName", &tHicName[0],"hicName/C");
  //outTree->Branch("chipNum", &tChipNum);
  //outTree->Branch("rowNum", &tRow);
  //outTree->Branch("colNum", &tCol);
  //outTree->Branch("thresh", &tThreshold, "thresh/s");
  //outTree->Branch("condVB", &tVbb);

  // addObject(outTree);

  hThresholdChipTemp = new TH1D("ThresholdTemp", "ThresholdTemp", 100, 0, 50);
  gm = o2::its::GeometryTGeo::Instance();
}

ITSQCTrhesholdTask::~ITSQCTrhesholdTask()
{
}

void ITSQCTrhesholdTask::initialize(o2::framework::InitContext& ctx)
{
  QcInfoLogger::GetInstance() << "initialize ITSQCTrhesholdTask" << AliceO2::InfoLogger::InfoLogger::endm;

  ptFileName = new TPaveText(0.20, 0.40, 0.85, 0.50, "NDC");
  ptFileName->SetTextSize(0.04);
  ptFileName->SetFillColor(0);
  ptFileName->SetTextAlign(12);
  ptFileName->AddText("Current File Proccessing: ");

  ptNFile = new TPaveText(0.20, 0.30, 0.85, 0.40, "NDC");
  ptNFile->SetTextSize(0.04);
  ptNFile->SetFillColor(0);
  ptNFile->SetTextAlign(12);
  ptNFile->AddText("File Processed: ");

  ptNEvent = new TPaveText(0.20, 0.20, 0.85, 0.30, "NDC");
  ptNEvent->SetTextSize(0.04);
  ptNEvent->SetFillColor(0);
  ptNEvent->SetTextAlign(12);
  ptNEvent->AddText("Event Processed: ");

  bulbRed = new TPaveText(0.60, 0.75, 0.90, 0.85, "NDC");
  bulbRed->SetTextSize(0.04);
  bulbRed->SetFillColor(0);
  bulbRed->SetTextAlign(12);
  bulbRed->SetTextColor(kRed);
  bulbRed->AddText("Red = QC Waiting");

  bulbYellow = new TPaveText(0.60, 0.65, 0.90, 0.75, "NDC");
  bulbYellow->SetTextSize(0.04);
  bulbYellow->SetFillColor(0);
  bulbYellow->SetTextAlign(12);
  bulbYellow->SetTextColor(kYellow);
  bulbYellow->AddText("Yellow = QC Pausing");

  bulbGreen = new TPaveText(0.60, 0.55, 0.90, 0.65, "NDC");
  bulbGreen->SetTextSize(0.04);
  bulbGreen->SetFillColor(0);
  bulbGreen->SetTextAlign(12);
  bulbGreen->SetTextColor(kGreen);
  bulbGreen->AddText("GREEN = QC Processing");

  hInfoCanvas->SetTitle("QC Process Information Canvas");
  hInfoCanvas->GetListOfFunctions()->Add(ptFileName);
  hInfoCanvas->GetListOfFunctions()->Add(ptNFile);
  hInfoCanvas->GetListOfFunctions()->Add(ptNEvent);
  hInfoCanvas->GetListOfFunctions()->Add(bulb);
  hInfoCanvas->GetListOfFunctions()->Add(bulbRed);
  hInfoCanvas->GetListOfFunctions()->Add(bulbYellow);
  hInfoCanvas->GetListOfFunctions()->Add(bulbGreen);

  publishHistos();

  cout << "DONE Inititing Publication = " << endl;
  bulb->SetFillColor(kRed);
  TotalFileDone = 0;
  TotalHisTime = 0;
  Counted = 0;
  Yellowed = 0;
}

void ITSQCTrhesholdTask::startOfActivity(Activity& activity)
{
  QcInfoLogger::GetInstance() << "startOfActivity" << AliceO2::InfoLogger::InfoLogger::endm;
}

void ITSQCTrhesholdTask::startOfCycle()
{
  QcInfoLogger::GetInstance() << "startOfCycle" << AliceO2::InfoLogger::InfoLogger::endm;
}

void ITSQCTrhesholdTask::monitorData(o2::framework::ProcessingContext& ctx)
{
  QcInfoLogger::GetInstance() << "START DOING QC General" << AliceO2::InfoLogger::InfoLogger::endm;

  int difference;
  std::chrono::time_point<std::chrono::high_resolution_clock> start;
  std::chrono::time_point<std::chrono::high_resolution_clock> startLoop;
  std::chrono::time_point<std::chrono::high_resolution_clock> end;

  start = std::chrono::high_resolution_clock::now();

  ofstream timefout("HisTimeGlobal.dat", ios::app);

  ofstream timefout2("HisTimeLoop.dat", ios::app);

  FileID = ctx.inputs().get<int>("File");
  int EPID = ctx.inputs().get<int>("EP");
  //cout<<"mFileInfo " <<ctx.inputs().get<int>("Finish")<<endl;
  getProcessStatus(ctx.inputs().get<int>("Finish"), FileFinish);
  cout << "___________________________________________________________________ mFileInfo " << ctx.inputs().get<int>("Finish") << " FileFinish " << FileFinish << endl;
  //	updateFile(ctx.inputs().get<int>("Run"), ctx.inputs().get<int>("EP"), FileID);

  //Will Fix Later//

  int ResetDecision = ctx.inputs().get<int>("in");
  QcInfoLogger::GetInstance() << "Reset Histogram Decision = " << ResetDecision << AliceO2::InfoLogger::InfoLogger::endm;
  if (ResetDecision == 1)
    reset();

  Int_t nRun = ctx.inputs().get<int>("Run");
  Errors = ctx.inputs().get<const std::array<unsigned int, NError>>("Error");

  for (int i = 0; i < NError; i++) {
    ErrorPerFile[i] = Errors[i] - ErrorPre[i];
  }

  for (int i = 0; i < NError; i++) {
    QcInfoLogger::GetInstance() << " i = " << i << "   Error = " << Errors[i] << "   ErrorPre = " << ErrorPre[i] << "   ErrorPerFile = " << ErrorPerFile[i] << AliceO2::InfoLogger::InfoLogger::endm;
    hErrorPlots->SetBinContent(i + 1, Errors[i]);
    hErrorFile->SetBinContent((FileID + 1 + (EPID - 4) * 12), i + 1, ErrorPerFile[i]);
  }

  if (FileFinish == 1) {
    for (int i = 0; i < NError; i++) {
      ErrorPre[i] = Errors[i];
    }
  }

  //	cout<<"*************************************************************Before Clusters"<<endl;
  //              auto clusters = ctx.inputs().get<gsl::span<Cluster>>("clusters");
  //    cout<<" +++++++++++++++++++++++++++++++ I HAVE RECIVEID CLUSTERS VECTOR WITH A SIZE: "<< clusters.size()<<endl;

  auto digits = ctx.inputs().get<gsl::span<o2::itsmft::Digit>>("digits"); //ruben suggestion WORKING!!!!!!!!!
  Int_t firstTime = 1;

  cout << "Digits were readed with size: " << digits.size() << "GBT counter= " << GBT_counter << endl;
  Int_t ChipID_prev;
  gm->fillMatrixCache(o2::utils::bit2Mask(o2::TransformType::L2G));
  Int_t newHistoAdded = 0;
  cout << "Before loop" << endl;
  /*
	for (auto&& pixeldata : digits) {
		ChipID = pixeldata.getChipIndex();
                    col = pixeldata.getColumn();
                    row = pixeldata.getRow();
		    gm->getChipId (ChipID, lay, sta, ssta, mod, chip);
		   cout<<"New File iteration: lay"<<lay<<": stave: "<<sta<< " chip " <<chip<<endl;


		break;
	}
*/

  //	vector<Int_t> vHic;
  Int_t newHic = 1;

  for (auto&& pixeldata : digits) {
    ChipID = pixeldata.getChipIndex();
    col = pixeldata.getColumn();
    row = pixeldata.getRow();
    if (ChipID != ChipID_prev)
      gm->getChipId(ChipID, lay, sta, ssta, mod, chip);
    //gm->getChipId (ChipID, lay, sta, ssta, mod, chip);
    // cout<< " [OB AID] 1 "<<endl;

    //----------------------------------------------------------------------------------------------------------
    if (vHic.size() > 0) {
      newHic = 1;
      for (int it = 0; it < vHic.size(); it++)
        if (vHic[it] == mod) {
          newHic = 0;
          break;
        }
      if (newHic == 1)
        vHic.push_back(mod);

    } else
      vHic.push_back(mod);
    //--------------------------------------------------------------------------------------------------------------------
    if (vLaySta.size() > 0) {

      Int_t newPair = 1;
      for (int it = 0; it < vLaySta.size(); it++)
        if (vLaySta[it].iLayer == lay && vLaySta[it].iStave == sta && vLaySta[it].iHic == mod) {
          newPair = 0;
          break;
        }
      if (newPair == 1) {
        struct EventParam st = { lay, sta, mod };

        vLaySta.push_back(st);
        cout << "added new pair, reason: new pair" << lay << ":" << sta << " : " << mod << " is not equal to old pairs " << endl;
        createLayerStaveHicHisto(lay, sta, mod, nRun);
        createLayerStaveHisto(lay, sta, nRun);
        //createLayerHisto(lay,nRun);
        newHistoAdded = 1;
      }

    } else {

      struct EventParam st = { lay, sta, mod };
      vLaySta.push_back(st);
      cout << "added new pair, reason, there is no pairs " << lay << ":" << sta << endl;
      createLayerStaveHicHisto(lay, sta, mod, nRun);
      createLayerStaveHisto(lay, sta, nRun);
      //createLayerHisto(lay,nRun);

      newHistoAdded = 1;
    }

    //	 cout<< " [OB AID] 2 "<<endl;

    //Do we have new chip?
    if (vChip.size() > 0) {
      Int_t newChip = 1;
      for (int it = 0; it < vChip.size(); it++)
        if (vChip[it] == chip) {
          newChip = 0;
          break;
        }
      if (newChip == 1)
        vChip.push_back(chip);

    } else
      vChip.push_back(chip);
    //	 cout<< " [OB AID] 3 "<<endl;

    int ChipNumber = (ChipID - ChipBoundary[lay]) - sta * NStaveChip[lay];
    int rowCS, colCS;
    getHicCoordinates(lay, chip, col, row, rowCS, colCS);

    //	 cout<< " [OB AID] 3.5 "<< "sta = "<<sta<<" lay= "<<lay << " mod= "<<mod<<endl;
    if (row >= 0 && col >= 0)
      hHitArray[sta][lay][mod][rowCS][colCS]++; //Need to check out, do we need -1?
    //if (colCS==9216) cout<< "////////////////////////////////////// at chip === 9 and ix = 9216, iy = "<<rowCS<<" we have hit!! "<<endl;
    ChipID_prev = ChipID;
    //	 cout<< " [OB AID] 4 "<<endl;
  }

  //FillHistos(1);
  cout << "*************************** I'm updating file" << endl;

  if (newHistoAdded == 1)
    updateFile(ctx.inputs().get<int>("Run"), ctx.inputs().get<int>("EP"), FileID);

  cout << "***********Done*************" << endl;

  /*
		 if (FileFinish==1) {
					GBT_counter++;
					cout<<"^^^^^^^^^^^^^^^^^^^ GBT file done!!!!!!!!!11^^^^^^^^^^^^^^^^^" <<endl;
		}
*/
  if (FileFinish == 1) {
    cout << "^^^^^^^^^^^^^^^^^^^ 3 GBT file done, time to Fill histos and clear histos ^^^^^^^^^^^^^ " << endl;
    for (int it = 0; it < vLaySta.size(); it++) {
      cout << "We have combination: Layer " << vLaySta[it].iLayer << " Stave " << vLaySta[it].iStave << " Hic " << vLaySta[it].iHic << endl;
    }
    for (int it = 0; it < vHic.size(); it++)
      FillHistos(lay, sta, vHic[it]);

    //FillHistos(lay,sta,mod);
    vHic.clear();
    vChip.clear();
    GBT_counter++;
  }

  end = std::chrono::high_resolution_clock::now();
  difference = std::chrono::duration_cast<std::chrono::milliseconds>(end - start).count();
  TotalHisTime = TotalHisTime + difference;
  QcInfoLogger::GetInstance() << "Time in Histogram = " << difference / 1000.0 << "s" << AliceO2::InfoLogger::InfoLogger::endm;
  timefout << "Time in Histogram = " << difference / 1000.0 << "s" << std::endl;

  if (NEvent == 0 && ChipID == 0 && row == 0 && col == 0 && Yellowed == 0) {
    bulb->SetFillColor(kYellow);
    Yellowed = 1;
  }
}

void ITSQCTrhesholdTask::FillHistos(Int_t iLayer, Int_t iStave, Int_t iHic)
{
  Int_t skipFirstRow;
  //cout<<"Insdie Fill Histos!!" << iLayer <<" " << iStave << " " << iChip<<endl;
  //for (int it = 0; it < vLaySta.size(); it++)
  {
    /* int iLayer=vLaySta[it].first;
			 			 int iStave=vLaySta[it].second;
						 ofstream SaveFile;
						 SaveFile.open ("fOUT_AID.txt",std::ios_base::app);
                                                 SaveFile<<"Inside Fill Histo with Stave "<<iStave << " Layer "<<iLayer<<endl;
						 SaveFile.close();
						*/
    Int_t columnBegin, columnEnd, DeadPixel;
    //hThreshold[iStave][iLayer]->Reset();
    //hThresholdChips[iStave][iLayer]->Reset();
    //hDeadPixelChips[iStave][iLayer]->Reset();
    //hHitDeadPixel[iStave][iLayer]->Reset();

    // for (Int_t iChip = 1; iChip<= NStaveChip[iLayer]/SizeReduce;iChip++)
    for (int it = 0; it < vChip.size(); it++) {
      cout << "______________________________________Begining, it = " << it << endl;
      Int_t iChip = vChip[it] + 1;
      cout << "Insdie Fill Histos!! iLayer" << iLayer << " iStave" << iStave << " iCHip" << iChip << " Chip Size " << vChip.size() << endl;

      DeadPixel = 0;
      columnBegin = NColHis * (iChip - 1);
      columnEnd = NColHis * iChip; //because last Row is Empty! (0-1022, 1023 - epmty, 1024 - 2046, 2047 - empty, 2048- 2070, etc)

      cout << "@@@@@@@@@@ columnBegin= " << columnBegin << " columnEnd= " << columnEnd << " GetNbinsY= " << hHitLay[iStave][iLayer][iHic]->GetNbinsY() << endl;
      for (Int_t iy = hHitLay[iStave][iLayer][iHic]->GetNbinsY(); iy >= 1; iy--) {

        for (Int_t ix = columnBegin; ix < columnEnd; ix++) {
          //	 cout<<"------ iy = "<<iy << " ix = "<<iy<<endl;
          Int_t Nhits = hHitArray[iStave][iLayer][iHic][iy - 1][ix];
          // hHitLay[iStave][iLayer][iHic]->SetBinContent(ix,iy,Nhits);
          thrInPixel = (1 - Nhits / 1250.) * 50;
          hThreshold[iStave][iLayer][iHic]->Fill(thrInPixel);
          hThresholdChipTemp->Fill(thrInPixel);
          //	 cout<<"still alive"<<endl;
          if (Nhits == 0) {
            DeadPixel++;
            Double_t DeadPixelCoordinate[2];
            DeadPixelCoordinate[0] = ix;
            DeadPixelCoordinate[1] = iy;
            //	if (iChip == 9) cout<<" !!!!!!!!!!!!!1 DeadPixel at x= "<<ix<<" y= "<<iy<<endl;
            //if (ix>1022 && ix<1026 && iy>300 && iy<305) cout<<"I'm filling ThnSparse! ix = "<<ix<<" iy= "<<iy<<endl;
            hHitDeadPixel[iStave][iLayer][iHic]->Fill(DeadPixelCoordinate);
          }
          //	cout<<"even now i'm alive"<<endl;

          TString tempHicName = Form("L%d_%d", iLayer, iStave);
          strncpy(tHicName, tempHicName.Data(), tempHicName.Sizeof());

          tChipNum = iChip;
          tRow = iy;
          tCol = ix - columnBegin;
          tThreshold = thrInPixel * 10;
          //	 tVbb = 103;

          //outTree->Fill();
          //	 cout<<"even now ... "<<endl;

        } //end of row loop
      }   // end of column
      cout << "*************** Filling huge histo" << endl;
      hThresholdChips[iStave][iLayer][iHic]->SetBinContent(iChip, hThresholdChipTemp->GetMean());
      hThresholdChips[iStave][iLayer][iHic]->SetBinError(iChip, hThresholdChipTemp->GetRMS());
      hDeadPixelChips[iStave][iLayer][iHic]->SetBinContent(iChip, DeadPixel);
      cout << " hDeadPixelChips[" << iStave << "][" << iLayer << "][" << iHic << "], iChip = " << iChip << " DeadPixelCount " << DeadPixel << endl;

      if (iLayer < 3) {
        hThresholdSummary_IB[iLayer]->SetBinContent(iChip, iStave + 1, hThresholdChipTemp->GetMean());
        hDeadPixelSummary_IB[iLayer]->SetBinContent(iChip, iStave + 1, DeadPixel);
      } else {

        hTresholdSummary_OB_Hic_vs_Chip[iLayer][iStave]->SetBinContent(iChip, iHic, hThresholdChipTemp->GetMean());
        hDeadPixelSummary_OB_Hic_vs_Chip[iLayer][iStave]->SetBinContent(iChip, iHic, DeadPixel);
      }

      hThresholdChipTemp->Reset();
    } //end of Chip loop

  } //end of Staves loop

  if (iLayer > 2) {

    cout << "============================================================================ iLayer " << iLayer << " iStave " << iStave + 1 << " iHic " << iHic << " ans " << hTresholdSummary_OB_Hic_vs_Chip[iLayer][iStave]->ProjectionX("_Threshold", iHic, iHic)->Integral() << endl;

    cout << "Check 1" << endl;
    if (hDeadPixelSummary_OB_Stave_vs_Hic[iLayer] == NULL)
      cout << "Problem with hDeadPixelSummary_OB_Stave_vs_Hic[iLayer]" << endl;
    if (hDeadPixelSummary_OB_Hic_vs_Chip[iLayer][iStave] == NULL)
      cout << "Problem with  hDeadPixelSummary_OB_Hic_vs_Chip[iLayer][iStave]" << endl;
    if (hDeadPixelSummary_OB_Hic_vs_Chip[iLayer][iStave]->ProjectionX("_D", iHic, iHic) == NULL)
      cout << " Problem with hDeadPixelSummary_OB_Hic_vs_Chip[iLayer][iStave]->ProjectionX(,iHic,iHic)" << endl;
    if (hTresholdSummary_OB_Stave_vs_Hic[iLayer] == NULL)
      cout << "Problem with hTresholdSummary_OB_Stave_vs_Hic[iLayer]" << endl;
    else
      cout << "All is fine with hTresholdSummary_OB_Stave_vs_Hic[iLayer]" << endl;
    cout << " All is fine but Im dumb" << endl;
    hDeadPixelSummary_OB_Stave_vs_Hic[iLayer]->SetBinContent(iHic, iStave + 1, hDeadPixelSummary_OB_Hic_vs_Chip[iLayer][iStave]->ProjectionX("_DeadPixel", iHic, iHic)->Integral());
    cout << "Check 2" << endl;
    Double_t ThresholdSummary = hTresholdSummary_OB_Hic_vs_Chip[iLayer][iStave]->ProjectionX("_Treshold", iHic, iHic)->Integral();
    hTresholdSummary_OB_Stave_vs_Hic[iLayer]->SetBinContent(iHic, iStave + 1, ThresholdSummary);

    cout << "Check 3" << endl;

    //
    //hTresholdSummary_OB_Stave_vs_Hic[iLayer]->SetBinContent(iStave+1,iHic, 2);
    //hTresholdSummary_OB_Stave_vs_Hic[iLayer]->SetBinContent(iStave+2,iHic+4, 10);
    //hTresholdSummary_OB_Stave_vs_Hic[iLayer]->SetBinContent(iStave-4,iHic-2, 5.1);

    //cout<<"++++++++++++++++++++++++++++++++++++ Content from Histo: " << hTresholdSummary_OB_Stave_vs_Hic[iLayer]->GetBinContent(iStave+1,iHic);
  }

  //					} //end of Layer loop

  //fListOut = new TFile (Form("ThresholdList_run%d.root",FileID),"RECREATE");
  //outTree->Write();
  //fListOut->Close();
  //delete fListOut;
}

void ITSQCTrhesholdTask::endOfCycle()
{

  QcInfoLogger::GetInstance() << "endOfCycle" << AliceO2::InfoLogger::InfoLogger::endm;
}

void ITSQCTrhesholdTask::endOfActivity(Activity& activity)
{
  QcInfoLogger::GetInstance() << "endOfActivity" << AliceO2::InfoLogger::InfoLogger::endm;
}

void ITSQCTrhesholdTask::reset()
{
  // clean all the monitor objects here

  QcInfoLogger::GetInstance() << "Resetting the histogram" << AliceO2::InfoLogger::InfoLogger::endm;

  TotalFileDone = 0;
  ptNFile->Clear();
  ptNFile->AddText(Form("File Processed: %d ", TotalFileDone));
  Yellowed = 0;
  for (Int_t iLayer = 0; iLayer < 3; iLayer++) {
    hThresholdSummary_IB[iLayer]->Reset();
    hDeadPixelSummary_IB[iLayer]->Reset();
  }
  /*
		 fListOut = new TFile (Form("ThresholdList_run%d.root",FileID),"RECREATE");
		 outTree->Write();
		 fListOut->Close();
                 delete fListOut;
*/
  //outTree->Reset();
}
//-----------------------------------------------------
//
//
//
//
//
//
//
/*
	   void ITSQCTrhesholdTask::createLayerHisto(Int_t iLayer, Int_t nRun){
		cout<<"Enter to createLayerHisto with iLayer= "<<iLayer<<endl;

                if (iLayer < NLayerIB) {
		

			if (hThresholdSummary_IB[iLayer] == NULL){


                		hThresholdSummary_IB[iLayer] = new TH2D( Form("Threshold/Layer%d/Threshold_Vs_Chip_and_Stave",iLayer),Form("ITS Layer%d, Threshold Vs Chip and Stave",iLayer),9,0,9,NStaves[iLayer],0,NStaves[iLayer]);
                		formatAxes(hThresholdSummary_IB[iLayer],"Chip Number","Stave Number",1,1.10);
                		hThresholdSummary_IB[iLayer]->SetTitle(Form("ITS Layer%d, Threshold Vs Chip and Stave",iLayer));
                		addObject(hThresholdSummary_IB[iLayer]);


                 		hDeadPixelSummary_IB[iLayer] = new TH2D( Form("DeadPixel/Layer%d/DeadPixel_Vs_Chip_and_Stave",iLayer),Form("ITS Layer%d, DeadPixel Vs Chip and Stave",iLayer),9,0,9,NStaves[iLayer],0,NStaves[iLayer]);
                 		formatAxes(hDeadPixelSummary_IB[iLayer],"Chip Number","Stave Number",1,1.10);
                 		hDeadPixelSummary_IB[iLayer]->SetTitle(Form("ITS Layer%d, DeadPixel Vs Chip and Stave",iLayer));
                 		addObject(hDeadPixelSummary_IB[iLayer]);


				cout<<"End of the create lay histo IB"<<endl;
			}
		}
		else{
			cout<<"Time to check wheter we have OB histograms"<<endl;
                	 if (hDeadPixelSummary_OB_Stave_vs_Hic[iLayer] == NULL){
			cout<<"We do not have them!!!"<<endl;

			//hDeadPixelSummary_OB_Stave_vs_Hic


                		hTresholdSummary_OB_Stave_vs_Hic[iLayer] = new TH2D( Form("Threshold/Layer%d/Threshold_Vs_Stave_and_Hic",iLayer),Form("ITS Layer%d, Threshold Vs Stave and Hic",iLayer),nHicPerStave[iLayer],0,nHicPerStave[iLayer],NStaves[iLayer],0,NStaves[iLayer]);
                		formatAxes(hTresholdSummary_OB_Stave_vs_Hic[iLayer],"HIC Number","Stave Number",1,1.10);
                		hTresholdSummary_OB_Stave_vs_Hic[iLayer]->SetTitle(Form("ITS Layer%d, Threshold  Stave VS HIC",iLayer));
                		addObject(hTresholdSummary_OB_Stave_vs_Hic[iLayer]);

                		hDeadPixelSummary_OB_Stave_vs_Hic[iLayer] = new TH2D( Form("DeadPixel/Layer%d/DeadPixel_Vs_Stave_and_Hic",iLayer),Form("ITS Layer%d, DeadPixel Vs Stave and Hic",iLayer),nHicPerStave[iLayer],0,nHicPerStave[iLayer],NStaves[iLayer],0,NStaves[iLayer]);
                		formatAxes(hDeadPixelSummary_OB_Stave_vs_Hic[iLayer],"HIC Number","Stave Number",1,1.10);
                		hDeadPixelSummary_OB_Stave_vs_Hic[iLayer]->SetTitle(Form("ITS Layer%d, DeadPixel Stave VS HIC",iLayer));
                		addObject(hDeadPixelSummary_OB_Stave_vs_Hic[iLayer]);
			
				getObjectsManager()->startPublishing( hTresholdSummary_OB_Stave_vs_Hic[iLayer]);
                         	getObjectsManager()->startPublishing(  hDeadPixelSummary_OB_Stave_vs_Hic[iLayer]);

                         	m_publishedObjects.push_back(  hTresholdSummary_OB_Stave_vs_Hic[iLayer]);
                         	m_publishedObjects.push_back(  hDeadPixelSummary_OB_Stave_vs_Hic[iLayer]);


				cout<<"End of the create lay histo OB"<<endl;
			}
			else cout<<"We have OB histo for this layer already"<<endl;


		}


	  }
*/

void ITSQCTrhesholdTask::createLayerStaveHisto(Int_t iLayer, Int_t iStave, Int_t nRun)
{

  if (iLayer > NLayerIB) {

    if (hTresholdSummary_OB_Hic_vs_Chip[iLayer][iStave] == NULL) {

      hTresholdSummary_OB_Hic_vs_Chip[iLayer][iStave] = new TH2D(Form("Threshold/Layer%d/Stave%d/Threshold_Hic_vs_Chip", iLayer, iStave), Form("ITS Layer%d Stave%d, Threshold Hic vs Chip", iLayer, iStave), 14, 0, 14, nHicPerStave[iLayer], 0, nHicPerStave[iLayer]);
      formatAxes(hTresholdSummary_OB_Hic_vs_Chip[iLayer][iStave], "Chip Number", "Hic Number", 1, 1.10);
      hTresholdSummary_OB_Hic_vs_Chip[iLayer][iStave]->SetTitle(Form("ITS Layer%d Stave%d, Threshold  HIC vs Chip", iLayer, iStave));
      addObject(hTresholdSummary_OB_Hic_vs_Chip[iLayer][iStave]);

      hDeadPixelSummary_OB_Hic_vs_Chip[iLayer][iStave] = new TH2D(Form("DeadPixel/Layer%d/Stave%d/DeadPixel_Hic_vs_Chip", iLayer, iStave), Form("ITS Layer%d Stave%d, DeadPixel Hic vs Chip", iLayer, iStave), 14, 0, 14, nHicPerStave[iLayer], 0, nHicPerStave[iLayer]);
      formatAxes(hDeadPixelSummary_OB_Hic_vs_Chip[iLayer][iStave], "Chip Number", "Hic Number", 1, 1.10);
      hDeadPixelSummary_OB_Hic_vs_Chip[iLayer][iStave]->SetTitle(Form("ITS Layer%d Stave%d, DeadPixel HIC vs Chip", iLayer, iStave));
      addObject(hDeadPixelSummary_OB_Hic_vs_Chip[iLayer][iStave]);

      cout << "End of the create lay sta histo" << endl;

      getObjectsManager()->startPublishing(hTresholdSummary_OB_Hic_vs_Chip[iLayer][iStave]);
      getObjectsManager()->startPublishing(hDeadPixelSummary_OB_Hic_vs_Chip[iLayer][iStave]);

      m_publishedObjects.push_back(hTresholdSummary_OB_Hic_vs_Chip[iLayer][iStave]);
      m_publishedObjects.push_back(hDeadPixelSummary_OB_Hic_vs_Chip[iLayer][iStave]);
    }
  }
}

void ITSQCTrhesholdTask::createLayerStaveHicHisto(Int_t iLayer, Int_t iStave, Int_t iHic, Int_t nRun)
{
  NChipLay[iLayer] = ChipBoundary[iLayer + 1] - ChipBoundary[iLayer];
  NStaveChip[iLayer] = NChipLay[iLayer] / NStaves[iLayer];

  if (hHitArray[iStave][iLayer][iHic] != NULL) {
    cout << "Histograms for layer " << iLayer << " stave " << iStave << " hic " << iHic << "  is already here! DELETING them!" << endl;
    delete hHitLay[iStave][iLayer][iHic];
    cout << "1" << endl;
    delete hHitArray[iStave][iLayer][iHic];
    cout << "2" << endl;
    delete hThreshold[iStave][iLayer][iHic];
    delete hThresholdChips[iStave][iLayer][iHic];
    cout << "3" << endl;
    delete hDeadPixelChips[iStave][iLayer][iHic];
    delete hHitDeadPixel[iStave][iLayer][iHic];
    cout << " End!" << endl;
  } else {
    cout << "No objects, start creation of the histos for sta" << iStave << " lay " << iLayer << " hic" << iHic << endl;
    if (iLayer < NLayerIB) {
      hHitArray[iStave][iLayer][iHic] = new unsigned short*[512];
      for (int i = 0; i < 512; ++i) {
        hHitArray[iStave][iLayer][iHic][i] = new unsigned short[9 * 1024];
        for (int j = 0; j < 9 * 1024; j++)
          hHitArray[iStave][iLayer][iHic][i][j] = 0;
      }
    } else {
      hHitArray[iStave][iLayer][iHic] = new unsigned short*[512 * 2];
      for (int i = 0; i < 512 * 2; ++i) {
        hHitArray[iStave][iLayer][iHic][i] = new unsigned short[7 * 1024];
        for (int j = 0; j < 7 * 1024; j++)
          hHitArray[iStave][iLayer][iHic][i][j] = 0;
      }
    }

    //--------------------------------------------------------------------

    //hitmaps histo
    int nBinsX, nBinsY, maxX, maxY, nChips;

    if (iLayer < NLayerIB) {
      maxX = 9 * NColHis;
      maxY = NRowHis;
      nChips = 9;
    } else {
      maxX = 7 * NColHis;
      maxY = 2 * NRowHis;
      nChips = 14;
    }
    nBinsX = maxX / SizeReduce;
    nBinsY = maxY / SizeReduce;

    NChipLay[iLayer] = ChipBoundary[iLayer + 1] - ChipBoundary[iLayer];
    NStaveChip[iLayer] = NChipLay[iLayer] / NStaves[iLayer];

    hHitLay[iStave][iLayer][iHic] = new TH2S(Form("Threshold/Layer%d/Stave%d/HIC%d/HITMAP", iLayer, iStave, iHic), Form("Layer%dStave%dHIC%dHITMAP", iLayer, iStave, iHic), nBinsX, 0, maxX, nBinsY, 0, nBinsY);
    formatAxes(hHitLay[iStave][iLayer][iHic], "Column", "Row", 1, 1.10);
    hHitLay[iStave][iLayer][iHic]->SetTitle(Form("Hits Map on Layer %d Stave %d HIC %d, Run000%d", iLayer, iStave, iHic, nRun));

    hThreshold[iStave][iLayer][iHic] = new TH1I(Form("Threshold/Layer%d/Stave%d/HIC%d/Threshold", iLayer, iStave, iHic), Form("Threshold%dStave%dHIC%d", iLayer, iStave, iHic), 100, 0, 50);
    formatAxes(hThreshold[iStave][iLayer][iHic], "Threshold, (DAC)", "Counts", 1, 1.10);
    hThreshold[iStave][iLayer][iHic]->SetTitle(Form("Threshold on Layer %d Stave %d HIC %d, Run000%d", iLayer, iStave, iHic, nRun));

    hThresholdChips[iStave][iLayer][iHic] = new TH1D(Form("Threshold/Layer%d/Stave%d/HIC%d/Threshold_CHIPS", iLayer, iStave, iHic), Form("Threshold%dStave%dHIC%d_CHIPS", iLayer, iStave, iHic), nChips, 0, nChips);
    formatAxes(hThresholdChips[iStave][iLayer][iHic], "Chip", "Threshold, (DAC)", 1, 1.10);
    hThresholdChips[iStave][iLayer][iHic]->SetTitle(Form("Threshold on Layer %d Stave %d HIC %d per Chip, Run000%d", iLayer, iStave, iHic, nRun));
    hDeadPixelChips[iStave][iLayer][iHic] = new TH1S(Form("DeadPixel/Layer%d/Stave%d/HIC%d/DeadPixel_CHIPS", iLayer, iStave, iHic), Form("DeadPixel%dStave%dHIC%d_CHIPS", iLayer, iStave, iHic), nChips, 0, nChips);
    formatAxes(hDeadPixelChips[iStave][iLayer][iHic], "Chip", "Dead Pixels (counts)", 1, 1.10);
    hDeadPixelChips[iStave][iLayer][iHic]->SetTitle(Form("DeadPixel on Layer %d Stave %d HIC %d per Chip, Run000%d", iLayer, iStave, iHic, nRun));

    Int_t bins[2] = { NColHis * NStaveChip[iLayer] / SizeReduce, NRowHis / SizeReduce };
    Double_t xmin[2] = { 0, 0 };
    Double_t xmax[2] = { maxX, maxY };
    hHitDeadPixel[iStave][iLayer][iHic] = new THnSparseD(Form("DeadPixel/Layer%d/Stave%d/HIC%d/DeadPixelHITMAP", iLayer, iStave, iHic), Form("Layer%dStave%dHIC%dDeadPixelHITMAP", iLayer, iStave, iHic), 2, bins, xmin, xmax);
    hHitDeadPixel[iStave][iLayer][iHic]->SetTitle(Form("Dead pixel Hits Map on Layer %d Stave %d HIC%d, Run000%d", iLayer, iStave, iHic, nRun));

    cout << "Adding objects for sta" << iStave << " iLayer " << iLayer << " HIC " << iHic << endl;
    // getObjectsManager()->startPublishing(hHitLay[iStave][iLayer][iHic]);
    getObjectsManager()->startPublishing(hThreshold[iStave][iLayer][iHic]);
    getObjectsManager()->startPublishing(hThresholdChips[iStave][iLayer][iHic]);
    getObjectsManager()->startPublishing(hDeadPixelChips[iStave][iLayer][iHic]);
    getObjectsManager()->startPublishing(hHitDeadPixel[iStave][iLayer][iHic]);

    //  m_publishedObjects.push_back(hHitLay[iStave][iLayer][iHic]);
    m_publishedObjects.push_back(hThreshold[iStave][iLayer][iHic]);
    m_publishedObjects.push_back(hThresholdChips[iStave][iLayer][iHic]);
    m_publishedObjects.push_back(hDeadPixelChips[iStave][iLayer][iHic]);
    m_publishedObjects.push_back(hHitDeadPixel[iStave][iLayer][iHic]);
  }

  cout << "End of the create lay sta hic histo" << endl;
}
/*
	  void  ITSQCTrhesholdTask::publishStaveLayerHisto(Int_t sta,Int_t lay){
		Int_t alreadyPublishing=0;

		for (unsigned int iObj = 0; iObj < m_publishedObjects.size(); iObj++){
			cout << "iObj = " <<iObj<<endl; 
			cout<<" I'm comparing: "<<hHitLay[sta][lay]->GetName() <<" with "<< m_publishedObjects.at(iObj)->GetName() << " and answer is "<<endl; 
                                                                if (hHitLay[sta][lay]->GetName() ==  m_publishedObjects.at(iObj)->GetName()){
                                                                        alreadyPublishing=1;
                                                                        cout<<"Already here objects for stave "<< sta<< " iLayer "<< lay<<endl;
                                                                        break;
                                                                }
		
		}
                                                if (alreadyPublishing==0){
									cout<<"Adding objects for sta"<< sta<< " iLayer "<< lay<<endl;
                                                                        getObjectsManager()->startPublishing(hHitLay[sta][lay]);
                                                                        getObjectsManager()->startPublishing(hThreshold[sta][lay]);
                                                                        getObjectsManager()->startPublishing(hThresholdChips[sta][lay]);
                                                                        getObjectsManager()->startPublishing(hDeadPixelChips[sta][lay]);

									m_publishedObjects.push_back(hHitLay[sta][lay]);
									m_publishedObjects.push_back(hThreshold[sta][lay]);
									m_publishedObjects.push_back(hThresholdChips[sta][lay]);
									m_publishedObjects.push_back(hDeadPixelChips[sta][lay]);

						}
	 }
*/
void ITSQCTrhesholdTask::getHicCoordinates(int aLayer, int aChip, int aCol, int aRow, int& aHicRow, int& aHicCol)
{
  aChip &= 0xf;
  if (aLayer < NLayerIB) {
    aHicCol = aChip * NCols + aCol;
    aHicRow = aRow;
  } else { // OB Hic: chip row 0 at center of HIC
    if (aChip < 7) {
      aHicCol = aChip * NCols + aCol;
      aHicRow = NRows - aRow - 1;
    } else {
      aHicRow = NRows + aRow;
      aHicCol = 7 * NCols - ((aChip - 8) * NCols + aCol);
    }
  }
}

void ITSQCTrhesholdTask::addObject(TObject* aObject, bool published)
{
  if (!aObject) {
    std::cout << "ERROR: trying to add non-existent object" << std::endl;
    return;
  }
  if (published) {
    m_publishedObjects.push_back(aObject);
  }
  m_objects.push_back(aObject);
}

void ITSQCTrhesholdTask::createGlobalHistos()
{
  hErrorPlots = new TH1D("General/ErrorPlots", "Decoding Errors", NError, 0.5, NError + 0.5);
  formatAxes(hErrorPlots, "Error ID", "Counts");
  hErrorPlots->SetMinimum(0);
  hErrorPlots->SetFillColor(kRed);

  hFileNameInfo = new TH1D("General/FileNameInfo", "FileNameInfo", 5, 0, 1);
  formatAxes(hFileNameInfo, "InputFile", "Total Files Processed", 1.1);

  hErrorFile = new TH2D("General/ErrorFile", "Decoding Errors vs File ID", NFiles + 1, -0.5, NFiles + 0.5, NError, 0.5, NError + 0.5);
  formatAxes(hErrorFile, "File ID (data-link)", "Error ID");
  //formatStatistics(hErrorFile);
  //format2DZaxis(hErrorFile);
  hErrorFile->GetZaxis()->SetTitle("Counts");
  hErrorFile->SetMinimum(0);

  hInfoCanvas = new TH1D("General/InfoCanvas", "InfoCanvas", 3, -0.5, 2.5);
  bulb = new TEllipse(0.2, 0.75, 0.30, 0.20);

  addObject(hErrorPlots);
  addObject(hFileNameInfo);
  addObject(hErrorFile);
  addObject(hInfoCanvas);
}
void ITSQCTrhesholdTask::formatAxes(TH1* h, const char* xTitle, const char* yTitle, float xOffset, float yOffset)
{
  h->GetXaxis()->SetTitle(xTitle);
  h->GetYaxis()->SetTitle(yTitle);
  h->GetXaxis()->SetTitleOffset(xOffset);
  h->GetYaxis()->SetTitleOffset(yOffset);
}

/* void ITSQCTrhesholdTask::formatStatistics(TH2 *h){
		h->SetStats(0);
     }*/
void ITSQCTrhesholdTask::formatStatistics(TH2* h, int low, int high)
{

  h->SetStats(0);
  h->GetZaxis()->SetRangeUser(low, high);
}

void ITSQCTrhesholdTask::publishHistos()
{
  for (unsigned int iObj = 0; iObj < m_publishedObjects.size(); iObj++) {
    getObjectsManager()->startPublishing(m_publishedObjects.at(iObj));
    cout << " [AID] Object will be published: " << m_publishedObjects.at(iObj)->GetName() << endl;
  }
}

void ITSQCTrhesholdTask::addMetadata(int runID, int EpID, int fileID)
{

  for (unsigned int iObj = 0; iObj < m_publishedObjects.size(); iObj++) {
    getObjectsManager()->addMetadata(m_publishedObjects.at(iObj)->GetName(), "Run", Form("%d", runID));
    getObjectsManager()->addMetadata(m_publishedObjects.at(iObj)->GetName(), "EP", Form("%d", EpID));
    getObjectsManager()->addMetadata(m_publishedObjects.at(iObj)->GetName(), "File", Form("%d", fileID));
  }
}

void ITSQCTrhesholdTask::getProcessStatus(int aInfoFile, int& aFileFinish)
{
  aFileFinish = aInfoFile % 10;
  //cout<<"aInfoFile = "<<aInfoFile<<endl;
  FileRest = (aInfoFile - aFileFinish) / 10;

  QcInfoLogger::GetInstance() << "FileFinish = " << aFileFinish << AliceO2::InfoLogger::InfoLogger::endm;
  QcInfoLogger::GetInstance() << "FileRest = " << FileRest << AliceO2::InfoLogger::InfoLogger::endm;

  if (aFileFinish == 0)
    bulb->SetFillColor(kGreen);
  if (aFileFinish == 1 && FileRest > 1)
    bulb->SetFillColor(kYellow);
  if (aFileFinish == 1 && FileRest == 1)
    bulb->SetFillColor(kRed);
}

void ITSQCTrhesholdTask::updateFile(int aRunID, int aEpID, int aFileID)
{

  static int RunIDPre = 0, FileIDPre = 0;
  if (RunIDPre != aRunID || FileIDPre != aFileID) {
    TString FileName = Form("infiles/run000%d/data-ep%d-link%d", aRunID, aEpID, aFileID);
    QcInfoLogger::GetInstance() << "For the Moment: RunID = " << aRunID << "  FileID = " << aFileID
                                << AliceO2::InfoLogger::InfoLogger::endm;
    hFileNameInfo->Fill(0.5);
    hFileNameInfo->SetTitle(Form("Current File Name: %s", FileName.Data()));
    TotalFileDone = TotalFileDone + 1;
    //hInfoCanvas->SetBinContent(1,FileID);
    //hInfoCanvas->SetBinContent(2,TotalFileDone);
    ptFileName->Clear();
    ptNFile->Clear();
    ptFileName->AddText(Form("File Being Proccessed: %s", FileName.Data()));
    ptNFile->AddText(Form("File Processed: %d ", TotalFileDone));
    cout << "[AID] Before Metadata Update" << endl;
    addMetadata(aRunID, aEpID, aFileID);
    cout << " [AID] After Metadata" << endl;
  }
  RunIDPre = aRunID;
  FileIDPre = aFileID;
}

} // namespace itsqctrhesholdtask
} // namespace quality_control_modules
} // namespace o2
