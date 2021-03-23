/*************************************************************************
 * Copyright (c) 2011 AT&T Intellectual Property 
 * All rights reserved. This program and the accompanying materials
 * are made available under the terms of the Eclipse Public License v1.0
 * which accompanies this distribution, and is available at
 * http://www.eclipse.org/legal/epl-v10.html
 *
 * Contributors: Details at https://graphviz.org
 *************************************************************************/

#ifdef __cplusplus
extern "C" {
#endif

#ifndef SPARSEGRAPH_H
#define SPARSEGRAPH_H

#include "config.h"

#ifdef USE_STYLES
    typedef enum { regular, invisible } Style;
#endif
    typedef struct {
	int nedges;		/* no. of neighbors, including self */
	int *edges;		/* edges[0..(nedges-1)] are neighbors; edges[0] is self */
	float *ewgts;		/* preferred edge lengths */
    } v_data; 

    typedef struct {
	int nedges;		/* no. of neighbors, including self */
	int *edges;		/* edges[0..(nedges-1)] are neighbors; edges[0] is self */
	float *ewgts;		/* preferred edge lengths */
	float *eweights;	/* edge weights */
#ifdef USE_STYLES
	Style *styles;
#endif
#ifdef DIGCOLA
	float *edists; /* directed dist reflecting the direction of the edge */
#endif
    } vtx_data;

    typedef int DistType;	/* must be signed!! */

extern void freeGraphData(vtx_data * graph);
extern void freeGraph(v_data * graph);

#endif

#ifdef __cplusplus
}
#endif
