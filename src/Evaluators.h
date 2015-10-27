#ifndef EVALUATORS_H_
#define EVALUATORS_H_

#include "DomineeringState.h"

#include <functional>
#include <utility>
#include <vector>

using DS = DomineeringState;

// TO DO: make helper methods.

struct EvalTakeAway {

    double operator()(const DS& state) const {
	int count = 0;

//	Either implement so it counts how many reserved are for opponent
//	or counts how many spaces it takes away from opponent
	// Counts how many reserved are for opponent
	 for (int i = 0; i < state.ROWS; i++){
                for (int j = 0; j < state.COLS; j++){
                        if (state.getCurPlayerSym() == 'B' && state.getCell(i,j) == state.EMPTYSYM){
                                if ((((j+1) < state.ROWS) && (state.getCell(i,j+1) == state.EMPTYSYM))){
                                         if (((i-1) > 0) && ((j+1) > 0) && (state.getCell(i-1,j) != state.EMPTYSYM) && (state.getCell(i-1,j+1) != state.EMPTYSYM) && (((i+1) > state.ROWS) || ((j+1) > state.COLS)) || (((i+1) < state.ROWS) && ((j+1) < state.COLS) && (state.getCell(i+1,j) != state.EMPTYSYM) && (state.getCell(i+1,j+1) != state.EMPTYSYM)&& (((i-1) < 0) || ((j+1 < 0))))){

                                                count++;
                                          }
                                          else if ((((i-1) > 0) && ((j+1) > 0) && (state.getCell(i-1,j) != state.EMPTYSYM) && (state.getCell(i-1,j+1) != state.EMPTYSYM)) && (((i+1) < state.ROWS) && ((j+1) < state.COLS) && (state.getCell(i+1,j) != state.EMPTYSYM) && (state.getCell(i+1,j+1) != state.EMPTYSYM))){
                                                count++;
                                          }
                                 }
                          }
                          else if (state.getCurPlayerSym() == 'W' && (state.getCell(i,j) == state.EMPTYSYM)){
                                         // See if adjacent square is open
                                if ((((i+1) < state.ROWS) && (state.getCell(i+1,j) == state.EMPTYSYM))){
                                         if ((((i+1) > 0) && ((j-1) > 0) && (state.getCell(i,j-1) != state.EMPTYSYM) && (state.getCell(i+1,j-1) != state.EMPTYSYM) && (((i+1) > state.ROWS) || ((j+1) > state.COLS))) || (((i+1) < state.ROWS) && ((j+1) < state.COLS) && (state.getCell(i,j+1) != state.EMPTYSYM) && (state.getCell(i+1,j+1) != state.EMPTYSYM)&& (((i-1) < 0) || ((j-1 < 0))))){

                                                count++;
                                          }
                                          else if ((((i+1) > 0) && ((j-1) > 0) && (state.getCell(i,j-1) != state.EMPTYSYM) && (state.getCell(i+1,j-1) != state.EMPTYSYM)) && (((i+1) < state.ROWS) && ((j+1) < state.COLS) && (state.getCell(i,j+1) != state.EMPTYSYM) && (state.getCell(i+1,j+1) != state.EMPTYSYM))){
                                                count++;
                                           }
                                }
                          }
                   }
        }
	

	// Counts how many spaces it takes away from opponent

	

	return count;
       // return u_dist(rd);
    }
};

struct EvalReserve {
    double operator()(const DS& state) const {
        int count = 0;

        // Loop through every square in the board, if homei

        for (int i = 0; i < state.ROWS; i++){
       	        for (int j = 0; j < state.COLS; j++){
			if (state.getCurPlayerSym() == 'W' && state.getCell(i,j) == state.EMPTYSYM){
			        if ((((j+1) < state.COLS) && (state.getCell(i,j+1) == state.EMPTYSYM))){
					 if (((i-1) > 0) && ((j+1) > 0) && (state.getCell(i-1,j) != state.EMPTYSYM) && (state.getCell(i-1,j+1) != state.EMPTYSYM) && (((i+1) > state.ROWS) || ((j+1) > state.COLS)) || (((i+1) < state.ROWS) && ((j+1) < state.COLS) && (state.getCell(i+1,j) != state.EMPTYSYM) && (state.getCell(i+1,j+1) != state.EMPTYSYM)&& (((i-1) < 0) || ((j+1 < 0))))){

						count++;
  					  }
					  else if ((((i-1) > 0) && ((j+1) > 0) && (state.getCell(i-1,j) != state.EMPTYSYM) && (state.getCell(i-1,j+1) != state.EMPTYSYM)) && (((i+1) < state.ROWS) && ((j+1) < state.COLS) && (state.getCell(i+1,j) != state.EMPTYSYM) && (state.getCell(i+1,j+1) != state.EMPTYSYM))){
						count++;
					  }
        		 	 }	
			  }
			  else if (state.getCurPlayerSym() == 'B' && (state.getCell(i,j) == state.EMPTYSYM)){
					 // See if adjacent square is open
	                  	if ((((i+1) < state.ROWS) && (state.getCell(i+1,j) == state.EMPTYSYM))){
                                         if ((((i+1) > 0) && ((j-1) > 0) && (state.getCell(i,j-1) != state.EMPTYSYM) && (state.getCell(i+1,j-1) != state.EMPTYSYM) && (((i+1) > state.ROWS) || ((j+1) > state.COLS))) || (((i+1) < state.ROWS) && ((j+1) < state.COLS) && (state.getCell(i,j+1) != state.EMPTYSYM) && (state.getCell(i+1,j+1) != state.EMPTYSYM)&& (((i-1) < 0) || ((j-1 < 0))))){

       	                                        count++;
                        	          }
					  else if ((((i+1) > 0) && ((j-1) > 0) && (state.getCell(i,j-1) != state.EMPTYSYM) && (state.getCell(i+1,j-1) != state.EMPTYSYM)) && (((i+1) < state.ROWS) && ((j+1) < state.COLS) && (state.getCell(i,j+1) != state.EMPTYSYM) && (state.getCell(i+1,j+1) != state.EMPTYSYM))){
                                                count++;
                                           }
                               	}
			  }	
       		   }
        }
//        return u_dist(rd);
	return count;
    }
};

using EvalScore = std::function<double(const DS&)>;
using EvalFactor = std::function<double(const DS&)>;
// vector of pairs because std::map requires operator< and std::unordered_map
// requires operator== and hash function for DomineeringState
static const std::vector<std::pair<EvalScore, EvalFactor>> evaluators = {
    { std::make_pair(EvalTakeAway(), [](const DS& state) { return 1; }) },
    { std::make_pair(EvalReserve(), [](const DS& state) { return 1; }) },
};

#endif /* end of include guard */

/* vim: tw=78:et:ts=4:sts=4:sw=4 */
