/*
 * flags.h
 *
 *  Created on: 26.09.2017
 *      Author: Daniel Höller
 */

#ifndef FLAGS_H_
#define FLAGS_H_


// constants
#define UNREACHABLE INT_MAX
#define NOACTION -1
#define FORBIDDEN -2

// [heuristics]
#define ZERO 0      // hVal is zero for every node
#define RCFILTER 1  // relaxed composition heuristic with the filter heuristic
#define RCFF 2      // relaxed composition heuristic with the FF heuristic
#define RCADD 3     // relaxed composition heuristic with the add heuristic
#define RCLMC 4     // relaxed composition heuristic with the LM-Cut heuristic
#define DOFREEILP 5 // delete and ordering free heuristic (exact ilp calculation)
#define DOFREELP 6  // delete and ordering free heuristic (approximate lp calculation)
#define LMCLOCAL 7
#define LMCANDOR 8

// [state-representation]
#define SRCOPY 0  // copy bit vector that represents state
#define SRCALC1 1 // calculate state based on current plan
#define SRCALC2 2 // calculate state based on current plan
#define SRLIST 3  // maintain int list with bits currently set

// [search-type]
#define DFSEARCH 0
#define BFSEARCH 1
#define HEURISTICSEARCH 2

// [algorithm]
#define PROGRESSIONORG 0 // branches over abstract and primitive tasks
#define ICAPS18 1        // branches over primitive and one abstract task
#define JAIR19 2         // processes abstract tasks first

// *****************
// * Configuration *
// *****************

// time limit in seconds
#define TIMELIMIT 600

//#define SEARCHALG PROGRESSIONORG
//#define SEARCHALG ICAPS18
#define SEARCHALG JAIR19 // choose from [algorithm]

#define EARLYGOALTEST

// select a state representation
#define STATEREP SRCOPY // choose from [state-representation]

#define DOFRECREATE 1
#define DOFUPDATE 2
#define DOFUPDATEWITHREACHABILITY 3



// type of search
#define SEARCHTYPE HEURISTICSEARCH // choose from [search-type]

// options for heuristic search
#define ASTAR
#define GASTARWEIGHT 1

#define PRGEFFECTLESS // always progress effectless actions

#define ONEMODAC
//#define ONEMODMETH

#define HEURISTIC LMCANDOR

#if ((HEURISTIC == LMCLOCAL) || (HEURISTIC == LMCANDOR))
#define TRACKLMS // track landmarks in the search nodes
#endif

#ifdef LMCANDORRA
#define MAINTAINREACHABILITY
#define ALLTASKS
#endif

#define CHECKAFTER 5000 // nodes after which the timelimit is checked
#ifndef OPTIMIZEUNTILTIMELIMIT
#define OPTIMIZEUNTILTIMELIMIT false
#endif

#ifndef CHECKAFTER
#define CHECKAFTER 5000 // nodes after which the timelimit is checked
#endif

#endif /* FLAGS_H_ */