//
/// \file   ITSQCTrhesholdTask.h
/// \author Barthelemy von Haller
/// \author Piotr Konopka
///

//#ifndef QC_MODULE_ITSQCTRHESHOLD_ITSQCTrhesholdTask_H
//#define QC_MODULE_ITSQCTRHESHOLD_ITSQCTrhesholdTask_H


#ifndef QC_MODULE_ITSQCTRHESHOLDTASK_ITSQCTRHESHOLDTASKITSQCTRHESHOLDTASK_H
#define QC_MODULE_ITSQCTRHESHOLDTASK_ITSQCTRHESHOLDTASKITSQCTRHESHOLDTASK_H



#include "QualityControl/TaskInterface.h"

#include <vector>
#include <deque>
#include <memory>
#include "Rtypes.h"		// for Digitizer::Class, Double_t, ClassDef, etc
#include "TObject.h"		// for TObject
#include "FairTask.h"
#include "TPaveText.h"
#include "TGaxis.h"
#include "TEllipse.h"
#include "TH1.h"
#include "TH2.h"
#include "THnSparse.h"
#include "ITSMFTReconstruction/RawPixelReader.h"


#include "DataFormatsITSMFT/ROFRecord.h"
#include "SimulationDataFormat/MCCompLabel.h"
#include <fstream>
#include "Framework/DataProcessorSpec.h"
#include "Framework/Task.h"
#include "DataFormatsITSMFT/Cluster.h"
#include "ITSMFTReconstruction/Clusterer.h"
//#include "uti.h"

//#include "ITSQCDataReaderWorkflow/QcDigit.h" //AID

#include "ITSBase/GeometryTGeo.h"
#include "DetectorsBase/GeometryManager.h"

#include "ITSMFTReconstruction/DigitPixelReader.h"
#include <ITSBase/GeometryTGeo.h>  //AID NEW 15.02.2020



#include "ITSMFTReconstruction/BuildTopologyDictionary.h"
#include "DataFormatsITSMFT/ClusterTopology.h"




class TH1F;

using namespace o2::quality_control::core;
using Cluster = o2::itsmft::Cluster;
using o2::itsmft::BuildTopologyDictionary;
namespace o2
{
	namespace quality_control_modules
	{
		namespace itsqctrhesholdtask
		{

			struct EventParam {
	
				Int_t iLayer, iStave, iHic;

			};





			/// \brief Example Quality Control DPL Task
			/// It is final because there is no reason to derive from it. Just remove it if needed.
			/// \author Barthelemy von Haller
			/// \author Piotr Konopka
			class ITSQCTrhesholdTask /*final*/ : public TaskInterface // todo add back the "final" when doxygen is fixed
			{
				public:
					/// \brief Constructor
					ITSQCTrhesholdTask();
					/// Destructor
					~ITSQCTrhesholdTask() override;

					// Definition of the methods for the template method pattern
					void initialize(o2::framework::InitContext& ctx) override;
					void startOfActivity(Activity& activity) override;
					void startOfCycle() override;
					void monitorData(o2::framework::ProcessingContext& ctx) override;
					void endOfCycle() override;
					void endOfActivity(Activity& activity) override;
					void reset() override;
					//void FillHistos(Int_t endOfFile, Int_t iLayer, Int_t iStave);  //AID
		//					void FillHistos(Int_t endOfFile);

				private:
					 TH1F* mHistogram = nullptr;

					void FillHistos(Int_t iLayer, Int_t iStave, Int_t iHic);
					void createGlobalHistos();
					void publishHistos();
					void addMetadata(int runID, int EpID, int fileID);
    					void formatAxes(TH1 *h, const char* xTitle, const char* yTitle, float xOffset = 1., float yOffset = 1.);
					//void formatAxes(TH2 *h, const char* xTitle, const char* yTitle, float xOffset = 1., float yOffset = 1.);
					void getProcessStatus (int aInfoFile, int& aFileFinish);
    					void updateFile (int aRunID, int aEpID,int aFileID);
					void addObject(TObject* aObject, bool published = true);
					//void formatStatistics(TH2 *h);
                                        void formatStatistics(TH2 *h, int low, int high);
					void getHicCoordinates (int aLayer, int aChip, int aCol, int aRow, int& aHicRow, int& aHicCol);
					void publishStaveLayerHisto(Int_t sta,Int_t lay);
					void createLayerStaveHicHisto(Int_t lay,Int_t sta, Int_t mod, Int_t nRun);
					void createLayerStaveHisto(Int_t lay,Int_t sta, Int_t nRun);
					void createLayerHisto(Int_t lay, Int_t nRun);
		
				//----------------
					TEllipse *bulb = new TEllipse(0.2,0.75,0.30,0.20);	
					TPaveText * ptFileName;
					TPaveText * ptNFile;
					TPaveText * ptNEvent;
					TPaveText * bulbGreen;
					TPaveText * bulbRed;
					TPaveText * bulbYellow;
					int ChipID; 
					int row;
					int col; 
					int NEvent;
					int RunIDPre;
					int FileIDPre;	
					int NEventPre;
					int GBT_counter=0;
					//int FileRest;
					int TotalFileDone;	
					 int Counted;
    					int TotalCounted = 10000;
    					int Yellowed;

				    	static constexpr int NCols = 1024;
    					static constexpr int NRows = 512;


   					 const int occUpdateFrequency = 1000000;
    
   					 int DivisionStep = 32;
    					static constexpr int NPixels = NRows * NCols;
    					static constexpr int NLayer = 7;
    					static constexpr int NLayerIB = 3;
                                       //------------
					
					TH1D *hErrorPlots;
    					TH1D *hFileNameInfo;
    					TH2D *hErrorFile;
					TH1D *hInfoCanvas;

					   std::vector<TObject*> m_objects;
    					std::vector<TObject*> m_publishedObjects;
					//---------------------------AID
					double ErrorMax;
			
					static constexpr int  NError = 11;
					TPaveText *pt[NError];
					TPaveText *ptTopology;
					std::array<unsigned int,NError> Errors;
					std::array<unsigned int,NError> ErrorPre;
					std::array<unsigned int,NError> ErrorPerFile;
				//	TH1D * ErrorPlots = new TH1D("ErrorPlots","ErrorPlots",NError,0.5,NError+0.5);
				    TString ErrorType[NError] ={"Error ID 1: ErrPageCounterDiscontinuity","Error ID 2: ErrRDHvsGBTHPageCnt","Error ID 3: ErrMissingGBTHeader","Error ID 4: ErrMissingGBTTrailer","Error ID 5: ErrNonZeroPageAfterStop","Error ID 6: ErrUnstoppedLanes","Error ID 7: ErrDataForStoppedLane","Error ID 8: ErrNoDataForActiveLane","Error ID 9: ErrIBChipLaneMismatch","Error ID 10: ErrCableDataHeadWrong","Error ID 11: Jump in RDH_packetCounter"};
					const int NFiles = 6;
					TH2I * ErrorFile = new TH2I("ErrorFile","ErrorFile",NFiles+1,-0.5,NFiles+0.5,NError,0.5,NError+0.5);
					
					std::vector<struct EventParam> vLaySta;
					std::vector<int> vChip;
					 vector<Int_t> vHic;
					TStopwatch watch1;
					o2::its::GeometryTGeo * gm;
					int lay, sta, ssta, mod, chip;
                    			long rowCS;
				        long  colCS;
					Int_t FileID;
					TFile *fListOut;
					TH2D *hThresholdSummary_IB[3]; 
					TH2D *hDeadPixelSummary_IB[3];
					TH2D *hTresholdSummary_OB_Stave_vs_Hic[7];
					TH2D *hTresholdSummary_OB_Hic_vs_Chip[7][48];
					TH2D *hDeadPixelSummary_OB_Stave_vs_Hic[7];
                                        TH2D *hDeadPixelSummary_OB_Hic_vs_Chip[7][48];
		


					
					TH2S * hHitLay[48][7][14];       //For now it's empty object, so we can create huge array
					TH1I * hThreshold[48][7][14];       //For now it's empty object, so we can create huge array
					TH1D * hThresholdChipTemp;
					TH1D *timeBetweenCycle;
					TH1D *timeToGetDigits;
					TH1D *timeAnalysis;
					TH1D * hThresholdChips[48][7][14];
					TH1S * hDeadPixelChips[48][7][14];
					THnSparse * hHitDeadPixel[48][7][14];

					TTree *outTree;
					Char_t tHicName[12];
					UChar_t tChipNum, tVbb;
					UShort_t tRow,tCol;
					UShort_t tThreshold;
//-------------------------------------------
				//short hHitArray[48][7][512][1024*9];
				//
					unsigned short **hHitArray[48][7][14];
					const int NLayers = 7;
					const int NStaves[7] = {12,16,20,24,30,42,48};  //AID for 8 stave dummy numbers
					int ChipBoundary[7 + 1] ={0,108,252,432,3120,6480,14712,24120};
					const int nHicPerStave[NLayer] = {1, 1, 1, 8, 8, 14, 14};
					const int nChipsPerHic[NLayer] = {9, 9, 9, 14, 14, 14, 14};

					int NChipLay[7];
					int NStaveChip[7];
//------------------------------------------------------------------------------------------------------
			//-------since that point doesnt work
					Double_t threshMean, threshRMS;
					const int NColHis = 1024;
				    const int NRowHis = 512;
					int SizeReduce = 1;
					int Nhits;
					Double_t thrInPixel;
					TH1D *hClusterSize;
					TH1D *hTopology;
				//---------------------clusters
			//		BuildTopologyDictionary completeDictionary;
				
			};

		} // namespace qcthresholds
	} // namespace quality_control_modules
} // namespace o2

#endif // QC_MODULE_QCGENERAL_ITSQCTrhesholdTask_H
