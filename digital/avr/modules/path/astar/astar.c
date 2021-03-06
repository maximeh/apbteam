/* astar.c */
/* avr.path.astar - A* path finding module. {{{
 *
 * Copyright (C) 2010 Nicolas Schodet
 *
 * APBTeam:
 *        Web: http://apbteam.org/
 *      Email: team AT apbteam DOT org
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA 02111-1307, USA.
 *
 * }}} */
#include "common.h"
#include "astar.h"

uint8_t
astar (struct astar_node_t *nodes, uint8_t nodes_nb, uint8_t initial,
       uint8_t goal)
{
    uint8_t i;
    /* Initialise all nodes. */
    for (i = 0; i < nodes_nb; i++)
      {
	nodes[i].score = ASTAR_NODE_SCORE_UNVISITED;
	nodes[i].heuristic = 0;
      }
    /* Start with the initial node. */
    nodes[initial].prev = 0xff; /* Not really read, no previous node. */
    nodes[initial].score = 0;
    nodes[initial].heuristic = AC_ASTAR_HEURISTIC_CALLBACK (initial);
    /* Loop until no node to consider. */
    while (1)
      {
	/* Find the node with the lowest total weight (score + heuristic) in
	 * the open set.  The score field special values has been chosen so
	 * that there is no extra test to select only nodes in the open set.
	 *
	 * If this code is too slow, this search should be replaced using a
	 * proper data structure (sorted list, heap...). */
	uint8_t lowest_node = 0;
	uint16_t lowest_weight = nodes[0].score + nodes[0].heuristic;
	for (i = 1; i < nodes_nb; i++)
	  {
	    if (nodes[i].score + nodes[i].heuristic < lowest_weight)
	      {
		lowest_node = i;
		lowest_weight = nodes[i].score + nodes[i].heuristic;
	      }
	  }
	/* If "found" node is not in the open set, there was no node found,
	 * abort. */
	uint16_t lowest_score = nodes[lowest_node].score;
	if (!ASTAR_NODE_SCORE_OPEN (lowest_score))
	    return 0;
	/* If this is the goal, report our success. */
	if (lowest_node == goal)
	    return 1;
	/* OK, there is some work, move this node to the closed set, it will
	 * never be consider again. */
	nodes[lowest_node].score = ASTAR_NODE_SCORE_CLOSED;
	nodes[lowest_node].heuristic = 0;
	/* Now, process all its neighbors. */
	struct astar_neighbor_t neighbors[nodes_nb - 1];
	uint8_t neighbors_nb = AC_ASTAR_NEIGHBOR_CALLBACK (lowest_node,
							   neighbors);
	for (i = 0; i < neighbors_nb; i++)
	  {
	    uint8_t neighbor = neighbors[i].node;
	    /* Never consider nodes in the closed set. */
	    if (nodes[neighbor].score == ASTAR_NODE_SCORE_CLOSED)
		continue;
	    /* See if our lowest_node is better to arrive to this neighbor
	     * node (node not considered yet, or new score is better).  Note
	     * that due to the score assigned to unvisited nodes, there is
	     * only one test. */
	    uint16_t tentative_score = lowest_score + neighbors[i].weight;
	    if (tentative_score < nodes[neighbor].score)
	      {
		nodes[neighbor].prev = lowest_node;
		/* Assign the new score, which as the side effect to put this
		 * node in the open set. */
		nodes[neighbor].score = tentative_score;
		/* Update heuristic if not done yet. */
		if (!nodes[neighbor].heuristic)
		    nodes[neighbor].heuristic =
			AC_ASTAR_HEURISTIC_CALLBACK (neighbor);
	      }
	  }
      }
}

/* Warning!
 * The heuristic field is used to known if a node has been visited. */
#define unvisited heuristic

void
astar_dijkstra_prepare (struct astar_node_t *nodes, uint8_t nodes_nb,
			uint8_t initial, uint8_t goal)
{
    /* Warning!
     * What is following is NOT the A* algorithm.  This look more like a
     * Dijkstra algorithm as every nodes are processed.  This is done because
     * goal node information (and therefore heuristic) is unknown for now. */
    uint8_t i;
    /* Initialise all nodes. */
    for (i = 0; i < nodes_nb; i++)
      {
	nodes[i].score = ASTAR_NODE_SCORE_UNVISITED;
	nodes[i].unvisited = 1;
      }
    /* Start with the initial node and explore every single node. */
    nodes[initial].prev = 0xff; /* Not really read, no previous node. */
    nodes[initial].score = 0;
    uint8_t lowest_node = initial;
    uint16_t lowest_score = 0;
    /* Loop until no node to consider. */
    while (1)
      {
	/* Mark the current node as visited, it will never be considered
	 * again. */
	nodes[lowest_node].unvisited = 0;
	/* Now, process all its neighbors. */
	struct astar_neighbor_t neighbors[nodes_nb - 1];
	uint8_t neighbors_nb = AC_ASTAR_NEIGHBOR_CALLBACK (lowest_node,
							   neighbors);
	for (i = 0; i < neighbors_nb; i++)
	  {
	    uint8_t neighbor = neighbors[i].node;
	    /* Never consider visited nodes. */
	    if (!nodes[neighbor].unvisited)
		continue;
	    /* See if the current node is better to arrive to this neighbor
	     * node. */
	    uint16_t tentative_score = lowest_score + neighbors[i].weight;
	    if (tentative_score < nodes[neighbor].score)
	      {
		nodes[neighbor].prev = lowest_node;
		nodes[neighbor].score = tentative_score;
	      }
	  }
	/* Find the next unvisited node with the lowest score. */
	lowest_node = 0xff;
	lowest_score = ASTAR_NODE_SCORE_UNVISITED;
	for (i = 0; i < nodes_nb; i++)
	  {
	    if (nodes[i].unvisited && i != goal
		&& nodes[i].score < lowest_score)
	      {
		lowest_node = i;
		lowest_score = nodes[i].score;
	      }
	  }
	/* Found node is visited or not reachable from initial node, abort. */
	if (lowest_node == 0xff)
	    break;
      }
}

uint16_t
astar_dijkstra_finish (struct astar_node_t *nodes, uint8_t nodes_nb,
		       uint8_t goal)
{
    uint8_t i;
    /* Find the shortest path from all of its neighbors. */
    struct astar_neighbor_t neighbors[nodes_nb - 1];
    uint8_t neighbors_nb = AC_ASTAR_NEIGHBOR_CALLBACK (goal, neighbors);
    uint16_t best_score = (uint16_t) -1;
    for (i = 0; i < neighbors_nb; i++)
      {
	uint8_t neighbor = neighbors[i].node;
	/* Only consider reachable neighbors. */
	if (ASTAR_NODE_SCORE_OPEN (nodes[neighbor].score))
	  {
	    uint16_t neighbor_score = nodes[neighbor].score
		+ neighbors[i].weight;
	    if (neighbor_score < best_score)
	      {
		best_score = neighbor_score;
		nodes[goal].prev = neighbor;
	      }
	  }
      }
    return best_score;
}

#undef unvisited

