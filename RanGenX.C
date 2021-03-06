#include <iostream>
#include <sys/time.h>

#include "TComplex.h"
#include "TProfile.h"
#include "TFile.h"
#include "TRandomGen.h"
#include "TRandom3.h"
#include "Recursion.C"

using namespace std;

const double pi= 3.1415926535;

// --- correlation function (generates particles, calls recursion)
void get_corr(int,int,int,double,int);

// --- recursion function (uses vector of angles to do calculations)
void do_recursion(vector<double>&,int);

void fix_ang(double&);

// --- gets system time, executes get_corr inside of over sequences/events
void execute(int,int,int,int,double,int,unsigned int);

void execute(int sequences, int nparticles, int ntuple, int harmonic, double space, int sequence_id, unsigned int seed)
{

  int stop = nparticles/ntuple;

  struct timeval Time;

  gettimeofday(&Time,0);
  int begintime = Time.tv_sec;
  //cout<<"begintime is "<<begintime<<endl;

  Init(nparticles); // initialize histograms

  //cout<<"Generating Correlations."<<endl;
  // --- generate the correlations
  //cout<<"Breaking around here."<<endl;
  for ( int j = 0; j < sequences; ++j )
    {
      if ( j % 10 == 0 ) cout << "Executing sequence j = " << j << endl;
      for ( int i = 1; i < stop; ++i )
	{
	  get_corr(i,ntuple,harmonic,space,seed);
	}
    }
  //cout<<"Done Generating Correlations."<<endl;
  // --- Use a time struct to get the current date and time to append to the output file names
  time_t t = time(NULL);
  struct tm tm = *localtime(&t);

  char timestamp[100];
  char daypart[100];
  char timepart[100];

  printf("now: %02d-%02d-%02d %02d:%02d:%02d\n", tm.tm_year + 1900, tm.tm_mon + 1, tm.tm_mday, tm.tm_hour, tm.tm_min, tm.tm_sec);
  sprintf(timepart,"%02d%02d", tm.tm_hour, tm.tm_min);
  sprintf(daypart,"%02d%02d%02d",tm.tm_year + 1900, tm.tm_mon + 1, tm.tm_mday);
  sprintf(timestamp,"%s-%s",daypart,timepart);
  cout<<"time stamp is "<<timestamp<<endl;

  char outfilename[100];
  sprintf(outfilename,"OutputFiles/OutFile_%s_h%d_k%d.root",timestamp,harmonic,ntuple);
  if ( sequence_id >= 0 ) sprintf(outfilename,"CondorOutput/OutFile_%04d_h%d_k%d.root",sequence_id,harmonic,ntuple);

  //--- make an output file to write the histograms
  TFile* HistFile = new TFile(outfilename,"recreate");
  HistFile->cd();
  cout<<"Writing Hists to File."<<endl;
  // --- write recursion histo
  for ( int cs = 0; cs < 2; ++cs )
    {
      for(int c = 0; c < maxCorrelator; ++c )
        {
	  hmult_recursion[cs][c]->Write();
	  d_hmult_recursion[cs][c]->Write();
        }
    }
  //cout<<"Writing File"<<endl;
  HistFile->Write();
  cout << "Wrote histogram file " << HistFile->GetName() << endl;
  HistFile->Close();
  delete HistFile;

  //cout<<"Deleting Hists."<<endl;
  Delete(); // delete histograms

  gettimeofday(&Time,0);
  int endtime = Time.tv_sec;
  //cout<<"endtime is "<<endtime<<endl;

  int tdiff = endtime-begintime;

  cout<<"End of program."<<endl;
  cout<<"Execution time: "<<tdiff<<" seconds"<<endl;

} //end of void RanGen2()



void get_corr(int nparticles, int ntuple, int harmonic, double space, int seed)
{

  if ( seed < 0 ) seed = 0;
  TRandom3 angle(seed);

  vector <double> ang; // inserting pairs into single object

  // int = integer, i can always be at 0,
  // i < 100 means go up to 100 events, ++i checks if it's less
  // than 100 and adds 1 -- if more than 100, than the program stops

  //int stop = nparticles/ntuple;
  //cout<<"Starting For loop over particles"<<endl;
  for ( int i = 0; i < nparticles; ++i )
    {
      //double means an exact number, phi1 is the name of the double for particle 1.
      double phi1 = angle.Uniform(-pi,pi);
      double increment = 2*pi/harmonic;
      double extra = angle.Uniform(-space,space);
      //double phi2 = 0; //same as above, but for particle 2
      //if ( phi1 > 0 ) phi2 = phi1 - 2*pi/harmonic;
      //if ( phi1 < 0 ) phi2 = phi1 + 2*pi/harmonic;
      double phi2 = phi1 + increment + extra;
      fix_ang (phi2);

      ang.push_back(phi1); //push_back to stack numbers on return
      ang.push_back(phi2);

      if ( ntuple <= 2 ) continue;
      
      extra = angle.Uniform(-space,space);
      double phi3 = phi2 + increment + extra;
      ang.push_back(phi3);
      if ( ntuple <= 3 ) continue;

      extra = angle.Uniform(-space,space);
      double phi4 = phi3 + increment + extra;
      ang.push_back(phi4);
      if ( ntuple <= 4 ) continue;

      extra = angle.Uniform(-space,space);      
      double phi5 = phi4 + increment + extra;
      ang.push_back(phi5);
      if ( ntuple <= 5 ) continue;

      extra = angle.Uniform(-space,space);
      double phi6 = phi5 + increment + extra;
      ang.push_back(phi6);
      if ( ntuple <= 6 ) continue;

      extra = angle.Uniform(-space,space);
      double phi7 = phi6 + increment + extra;
      ang.push_back(phi7);
      if ( ntuple <= 7 ) continue;

      extra = angle.Uniform(-space,space);
      double phi8 = phi7 + increment + extra;
      ang.push_back(phi8);
      if ( ntuple <= 8 ) continue;


    } // end of nparticles for loop
  //cout<<"Done Generating Particles, doing Recursion"<<endl;
  do_recursion(ang,harmonic);
  //cout<<"Done with Recursion"<<endl;
  
  return;

} //end of get_corr

void fix_ang(double& ang)
{
  if (ang > pi) ang -= 2 * pi;
  if (ang < -pi) ang += 2 * pi;
}

void do_recursion(vector<double>& ang, int harmonic)
{

  for(int h=0;h<maxHarmonic;h++)
    {
      for(int w=0;w<maxPower;w++)
        {
          Qvector[h][w] = TComplex(0.,0.); // initialize Q-vector to zero
        } // end of loop over Qvector
    } // end of loop over maxHarmonic

  int mult = ang.size();

  for ( int i = 0; i < mult; ++i )
    {
      //cout << ang[i] << " " ;
      for(int h=0;h<maxHarmonic;h++)
        {
          double phi = ang[i];
          // do the summation for the Q-vectors
          for(int w=0;w<maxPower;w++)
            {
              Qvector[h][w] += TComplex(cos(h*phi),sin(h*phi));
            } // end of loop over powers
        } // end of loop over harmonics
    } // end of loop over ang vector

  // --- from generic formulas ----------------------------------------------------------------------------

  //  2-p correlations
  int harmonics_Two_Num[2] = {harmonic,-harmonic}; // harmonic*(1,-1)
  int harmonics_Two_Den[2] = {0,0}; // recursion gives the right combinatorics
  TComplex twoRecursion = Recursion(2,harmonics_Two_Num)/Recursion(2,harmonics_Two_Den).Re();
  //double spwTwoRecursion = Recursion(2,harmonics_Two_Den).Re();
  double wTwoRecursion = 1.0;
  hmult_recursion[0][0]->Fill(mult,twoRecursion.Re(),wTwoRecursion);
  hmult_recursion[1][0]->Fill(mult,twoRecursion.Im(),wTwoRecursion);

  //  3-p correlations
  int harmonics_Three_Num[3] = {harmonic,harmonic,-2*harmonic}; // harmonic*(1,1,-2)
  int harmonics_Three_Den[3] = {0,0,0}; // recursion gives the right combinatorics
  TComplex threeRecursion = Recursion(3,harmonics_Three_Num)/Recursion(3,harmonics_Three_Den).Re();
  //double spwThreeRecursion = Recursion(3,harmonics_Three_Den).Re();
  double wThreeRecursion = 1.0;
  hmult_recursion[0][1]->Fill(mult,threeRecursion.Re(),wThreeRecursion);
  hmult_recursion[1][1]->Fill(mult,threeRecursion.Im(),wThreeRecursion);

  //  4-p correlations
  int harmonics_Four_Num[4] = {harmonic,harmonic,-harmonic,-harmonic}; // harmonic*(1,1,-1,-1)
  int harmonics_Four_Den[4] = {0,0,0,0};
  TComplex fourRecursion = Recursion(4,harmonics_Four_Num)/Recursion(4,harmonics_Four_Den).Re();
  //double spwFourRecursion = Recursion(4,harmonics_Four_Den).Re();
  double wFourRecursion = 1.0;
  hmult_recursion[0][2]->Fill(mult,fourRecursion.Re(),wFourRecursion);
  hmult_recursion[1][2]->Fill(mult,fourRecursion.Im(),wFourRecursion);

  //  5-p correlations
  int harmonics_Five_Num[5] = {harmonic,harmonic,harmonic,-harmonic,-2*harmonic}; // harmonic*(1,1,1,-1,-2)
  int harmonics_Five_Den[5] = {0,0,0,0,0}; // recursion gives the right combinatorics
  TComplex fiveRecursion = Recursion(5,harmonics_Five_Num)/Recursion(5,harmonics_Five_Den).Re();
  //double spwFiveRecursion = Recursion(5,harmonics_Five_Den).Re();
  double wFiveRecursion = 1.0;
  hmult_recursion[0][3]->Fill(mult,fiveRecursion.Re(),wFiveRecursion);
  hmult_recursion[1][3]->Fill(mult,fiveRecursion.Im(),wFiveRecursion);

  //  6-p correlations:
  int harmonics_Six_Num[6] = {harmonic,harmonic,harmonic,-harmonic,-harmonic,-harmonic}; // harmonic*(1,1,1,-1,-1,-1)
  int harmonics_Six_Den[6] = {0,0,0,0,0,0};
  TComplex sixRecursion = Recursion(6,harmonics_Six_Num)/Recursion(6,harmonics_Six_Den).Re();
  //double spwSixRecursion = Recursion(6,harmonics_Six_Den).Re();
  double wSixRecursion = 1.0;
  hmult_recursion[0][4]->Fill(mult,sixRecursion.Re(),wSixRecursion);
  hmult_recursion[1][4]->Fill(mult,sixRecursion.Im(),wSixRecursion);

  //  7-p correlations
  int harmonics_Seven_Num[7] = {harmonic,harmonic,harmonic,harmonic,-harmonic,-harmonic,-2*harmonic}; // harmonic*(1,1,1,1,-1,-1,-2)
  int harmonics_Seven_Den[7] = {0,0,0,0,0,0,0}; // recursion gives the right combinatorics
  TComplex sevenRecursion = Recursion(7,harmonics_Seven_Num)/Recursion(7,harmonics_Seven_Den).Re();
  //double spwSevenRecursion = Recursion(7,harmonics_Seven_Den).Re();
  double wSevenRecursion = 1.0;
  hmult_recursion[0][5]->Fill(mult,sevenRecursion.Re(),wSevenRecursion);
  hmult_recursion[1][5]->Fill(mult,sevenRecursion.Im(),wSevenRecursion);

  //  8-p correlations
  int harmonics_Eight_Num[8] = {harmonic,harmonic,harmonic,harmonic,-harmonic,-harmonic,-harmonic,-harmonic};
  int harmonics_Eight_Den[8] = {0,0,0,0,0,0,0,0};
  TComplex eightRecursion = Recursion(8,harmonics_Eight_Num)/Recursion(8,harmonics_Eight_Den).Re();
  //double spwEightRecursion = Recursion(8,harmonics_Eight_Den).Re();
  double wEightRecursion = 1.0;
  hmult_recursion[0][6]->Fill(mult,eightRecursion.Re(),wEightRecursion);
  hmult_recursion[1][6]->Fill(mult,eightRecursion.Im(),wEightRecursion);



  // ---------------------------------------------------------------------------------------
  // ---------------------------------------------------------------------------------------
  // ---------------------------------------------------------------------------------------



  int d_harmonic = 2*harmonic;
  
  //  2-p correlations
  int d_harmonics_Two_Num[2] = {d_harmonic,-d_harmonic}; // d_harmonic*(1,-1)
  int d_harmonics_Two_Den[2] = {0,0}; // recursion gives the right combinatorics
  TComplex twod_Recursion = Recursion(2,d_harmonics_Two_Num)/Recursion(2,d_harmonics_Two_Den).Re();
  //double spwTwod_Recursion = Recursion(2,d_harmonics_Two_Den).Re();
  double wTwod_Recursion = 1.0;
  d_hmult_recursion[0][0]->Fill(mult,twod_Recursion.Re(),wTwod_Recursion);
  d_hmult_recursion[1][0]->Fill(mult,twod_Recursion.Im(),wTwod_Recursion);

  //  3-p correlations
  int d_harmonics_Three_Num[3] = {d_harmonic,d_harmonic,-2*d_harmonic}; // d_harmonic*(1,1,-2)
  int d_harmonics_Three_Den[3] = {0,0,0}; // recursion gives the right combinatorics
  TComplex threed_Recursion = Recursion(3,d_harmonics_Three_Num)/Recursion(3,d_harmonics_Three_Den).Re();
  //double spwThreed_Recursion = Recursion(3,d_harmonics_Three_Den).Re();
  double wThreed_Recursion = 1.0;
  d_hmult_recursion[0][1]->Fill(mult,threed_Recursion.Re(),wThreed_Recursion);
  d_hmult_recursion[1][1]->Fill(mult,threed_Recursion.Im(),wThreed_Recursion);

  //  4-p correlations
  int d_harmonics_Four_Num[4] = {d_harmonic,d_harmonic,-d_harmonic,-d_harmonic}; // d_harmonic*(1,1,-1,-1)
  int d_harmonics_Four_Den[4] = {0,0,0,0};
  TComplex fourd_Recursion = Recursion(4,d_harmonics_Four_Num)/Recursion(4,d_harmonics_Four_Den).Re();
  //double spwFourd_Recursion = Recursion(4,d_harmonics_Four_Den).Re();
  double wFourd_Recursion = 1.0;
  d_hmult_recursion[0][2]->Fill(mult,fourd_Recursion.Re(),wFourd_Recursion);
  d_hmult_recursion[1][2]->Fill(mult,fourd_Recursion.Im(),wFourd_Recursion);

  //  5-p correlations
  int d_harmonics_Five_Num[5] = {d_harmonic,d_harmonic,d_harmonic,-d_harmonic,-2*d_harmonic}; // d_harmonic*(1,1,1,-1,-2)
  int d_harmonics_Five_Den[5] = {0,0,0,0,0}; // recursion gives the right combinatorics
  TComplex fived_Recursion = Recursion(5,d_harmonics_Five_Num)/Recursion(5,d_harmonics_Five_Den).Re();
  //double spwFived_Recursion = Recursion(5,d_harmonics_Five_Den).Re();
  double wFived_Recursion = 1.0;
  d_hmult_recursion[0][3]->Fill(mult,fived_Recursion.Re(),wFived_Recursion);
  d_hmult_recursion[1][3]->Fill(mult,fived_Recursion.Im(),wFived_Recursion);

  //  6-p correlations:
  int d_harmonics_Six_Num[6] = {d_harmonic,d_harmonic,d_harmonic,-d_harmonic,-d_harmonic,-d_harmonic}; // d_harmonic*(1,1,1,-1,-1,-1)
  int d_harmonics_Six_Den[6] = {0,0,0,0,0,0};
  TComplex sixd_Recursion = Recursion(6,d_harmonics_Six_Num)/Recursion(6,d_harmonics_Six_Den).Re();
  //double spwSixd_Recursion = Recursion(6,d_harmonics_Six_Den).Re();
  double wSixd_Recursion = 1.0;
  d_hmult_recursion[0][4]->Fill(mult,sixd_Recursion.Re(),wSixd_Recursion);
  d_hmult_recursion[1][4]->Fill(mult,sixd_Recursion.Im(),wSixd_Recursion);

  //  7-p correlations
  int d_harmonics_Seven_Num[7] = {d_harmonic,d_harmonic,d_harmonic,d_harmonic,-d_harmonic,-d_harmonic,-2*d_harmonic}; // d_harmonic*(1,1,1,1,-1,-1,-2)
  int d_harmonics_Seven_Den[7] = {0,0,0,0,0,0,0}; // recursion gives the right combinatorics
  TComplex sevend_Recursion = Recursion(7,d_harmonics_Seven_Num)/Recursion(7,d_harmonics_Seven_Den).Re();
  //double spwSevend_Recursion = Recursion(7,d_harmonics_Seven_Den).Re();
  double wSevend_Recursion = 1.0;
  d_hmult_recursion[0][5]->Fill(mult,sevend_Recursion.Re(),wSevend_Recursion);
  d_hmult_recursion[1][5]->Fill(mult,sevend_Recursion.Im(),wSevend_Recursion);

  //  8-p correlations
  int d_harmonics_Eight_Num[8] = {d_harmonic,d_harmonic,d_harmonic,d_harmonic,-d_harmonic,-d_harmonic,-d_harmonic,-d_harmonic};
  int d_harmonics_Eight_Den[8] = {0,0,0,0,0,0,0,0};
  TComplex eightd_Recursion = Recursion(8,d_harmonics_Eight_Num)/Recursion(8,d_harmonics_Eight_Den).Re();
  //double spwEightd_Recursion = Recursion(8,d_harmonics_Eight_Den).Re();
  double wEightd_Recursion = 1.0;
  d_hmult_recursion[0][6]->Fill(mult,eightd_Recursion.Re(),wEightd_Recursion);
  d_hmult_recursion[1][6]->Fill(mult,eightd_Recursion.Im(),wEightd_Recursion);

  // --- print statements for diagnostic purposes
  //cout << twod_Recursion.Re() << endl;
  //cout << fourd_Recursion.Re() << endl;
  //cout << sixd_Recursion.Re() << endl;
  //cout << eightd_Recursion.Re() << endl;

} // end do_recursion functions
