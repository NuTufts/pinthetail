#include <iostream>
#include <string>

// larlite
#include "DataFormat/mctrack.h"
#include "DataFormat/mctrajectory.h"

// larcv
#include "DataFormat/EventImage2D.h"
#include "DataFormat/EventROI.h"
#include "DataFormat/Image2D.h"

// llcv
#include "Base/DataCoordinator.h"

// opencv
#include <opencv2/opencv.hpp>
#include <opencv2/core/core.hpp>

int main( int nargs, char** argv ) {

  std::cout << "[ Select Tune Sample ]" << std::endl;

  std::string SUP_FILE   = argv[1];  // supera
  std::string MC_FILE    = argv[2];  // mcinfo
  std::string OUT_FILE   = argv[3];

  // holds images and mcinfo trees per event
  larlitecv::DataCoordinator dataco;
  dataco.add_inputfile( MC_FILE,  "larlite" );
  dataco.add_inputfile( SUP_FILE, "larcv" );  
  dataco.initialize();

  // output for larcv information
  larcv::IOManager outlarcv( larcv::IOManager::kWRITE );
  outlarcv.set_out_file( OUT_FILE );
  outlarcv.initialize();
  
  int nentries = dataco.get_nentries( "larcv" );
  nentries = 3;
  
  for ( int i=0; i<nentries; i++) {
    dataco.goto_entry(i,"larcv");

    // get rse
    int run;
    int subrun;
    int event;
    dataco.get_id( run, subrun, event );
    
    std::cout << "entry " << i << " (" << run << "," << subrun << "," << event << ")" << std::endl;

    // adc images (event-wise)
    larcv::EventImage2D* ev_img_v = (larcv::EventImage2D*)dataco.get_larcv_data( larcv::kProductImage2D, "wire" );

    // instance ID images
    larcv::EventImage2D* ev_id_v  = (larcv::EventImage2D*)dataco.get_larcv_data( larcv::kProductImage2D, "instance" );
    larcv::EventImage2D* ev_mom_v = (larcv::EventImage2D*)dataco.get_larcv_data( larcv::kProductImage2D, "ancestor" );

    // mctrack/mcshower info
    larlite::event_mctrack* ev_tracks   = (larlite::event_mctrack*)dataco.get_larlite_data( larlite::data::kMCTrack, "mcreco" );
    larlite::event_mcshower* ev_showers = (larlite::event_mcshower*)dataco.get_larlite_data( larlite::data::kMCShower, "mcreco" );

    // croi ROIs
    larcv::EventROI* ev_croi_v = (larcv::EventROI*)dataco.get_larcv_data( larcv::kProductROI, "croi" );

    const std::vector<larcv::Image2D>& img_v = ev_img_v->Image2DArray();
    const std::vector<larcv::ROI>& croi_v = ev_croi_v->ROIArray();
    
    if ( img_v.size()==0 ) {
      std::cout << "No image in event" << std::endl;
      continue;
    }
    if ( croi_v.size()==0 ) {
      std::cout << "No CROI in event" << std::endl;
      continue;
    }

    // we gather the points we want to label
    // get cosmics: origin
    std::vector< std::vector<float> > cosmic_muon_start;
    std::vector< std::vector<float> > cosmic_muon_end;
    std::vector<float> neutrino_start(3);

    for ( auto const& mctrack : *ev_tracks ) {
      // std::cout << "origin=" << mctrack.Origin()
      // 		<< " Mother Track: " << mctrack.TrackID() << " " << mctrack.MotherTrackID() << std::endl;
      if ( mctrack.Origin()==1 ) {
	if ( mctrack.MotherTrackID()==mctrack.TrackID() ) {
	  for (int i=0; i<3; i++)
	    neutrino_start[i] = mctrack.Start().Position()(i);
	}
      }
      else if ( mctrack.Origin()==2 && (mctrack.PdgCode()==13 || mctrack.PdgCode()==-13) && mctrack.TrackID()==mctrack.MotherTrackID() ) {
	// muon start
	std::vector<float> start(3);
	std::vector<float> end(3);
	for (int i=0; i<3; i++) {
	  start[i] = mctrack.Start().Position()(i);
	  end[i]   = mctrack.End().Position()(i);
	}
	cosmic_muon_start.push_back( start );
	cosmic_muon_end.push_back( end );
      }
    }

    std::cout << "MC vertex: "
	      << "(" << neutrino_start[0] << ","
	      << neutrino_start[1] << ","
	      << neutrino_start[2] << ")"
	      << std::endl;
    std::cout << "muon start points=" << cosmic_muon_start.size() << std::endl;
    std::cout << "muon end points=" << cosmic_muon_end.size() << std::endl;

    int nroi = croi_v.size();
    std::cout << "Number of ROIs:" << nroi << std::endl;
    
    /*
    // crop and save
    int nroi = roi_v.size();
    if ( nroi>10 )
      nroi = 10;
    for ( int iroi=0; iroi<nroi; iroi++) {

      // set the output event container
      larcv::EventImage2D* ev_out    = (larcv::EventImage2D*)outlarcv.get_data( larcv::kProductImage2D, "adc" );
      larcv::EventImage2D* ev_label  = (larcv::EventImage2D*)outlarcv.get_data( larcv::kProductImage2D, "label" );
      larcv::EventImage2D* ev_weight = (larcv::EventImage2D*)outlarcv.get_data( larcv::kProductImage2D, "weight" );      

      larcv::ROI& roi = roi_v.at(iroi);

      //std::cout << roi.dump() << std::endl;
      //std::cout << "pixelwidth: " << roi.BB(0).pixel_width() << " " << roi.BB(0).pixel_height() << std::endl;
      
      // crop the image
      for ( auto const& img : img_v ) {

	larcv::Image2D cropped = img.crop( roi.BB( img.meta().plane() ) );	
	ev_out->Emplace( std::move(cropped) );
      }

      std::vector<larcv::Image2D> label_v;
      std::vector<larcv::Image2D> weight_v;      
      make_cropped_label_image( ev_out->Image2DArray(), ev_id_v->Image2DArray(), ev_mom_v->Image2DArray(),
				*ev_tracks, *ev_showers, 1.0, label_v, weight_v );
				
      ev_label->Emplace( std::move(label_v) );
      ev_weight->Emplace( std::move( weight_v) );
      
      // set the entry number
      outlarcv.set_id( run, subrun, event*10 + iroi );

      outlarcv.save_entry();
      
    }
    */
  }
  
  outlarcv.finalize();

  return 0;
}
