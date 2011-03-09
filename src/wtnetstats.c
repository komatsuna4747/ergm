#include "wtnetstats.h"
/*****************
 void network_stats_wrapper

 Wrapper for a call from R.  Return the change in the statistics when
 we go from an empty graph to the observed graph.  If the empty graph
 has true global values equal to zero for all statistics, then this
 change gives the true global values for the observed graph.
*****************/
void wt_network_stats_wrapper(int *heads, int *tails, double *weights, int *dnedges,
			   int *dn, int *dflag,  int *bipartite,
			   int *nterms, char **funnames,
			   char **sonames, double *inputs,  double *stats)
{
  int directed_flag, hammingterm;
  Vertex n_nodes;
  Edge n_edges, nddyads;
  WtNetwork nw[2];
  WtModel *m;
  WtModelTerm *thisterm;
  Vertex bip;

/*	     Rprintf("prestart with setup\n"); */
  n_nodes = (Vertex)*dn; 
  n_edges = (Edge)*dnedges;     
  directed_flag = *dflag;
  bip = (Vertex)*bipartite;
  
  m=WtModelInitialize(*funnames, *sonames, &inputs, *nterms);
  nw[0]=WtNetworkInitialize(NULL, NULL, NULL, 0,
			    n_nodes, directed_flag, bip, 0);

  /* Compute the change statistics and copy them to stats for return to R. */
  WtSummStats(n_edges, heads, tails, weights, nw, m,stats);
  
  WtModelDestroy(m);
  WtNetworkDestroy(nw);
}


/****************
 void SummStats Computes summary statistics for a network. Must be
 passed an empty network (and a possible discordance network) and 
 passed an empty network
*****************/
void WtSummStats(Edge n_edges, Vertex *heads, Vertex *tails, double *weights,
WtNetwork *nwp, WtModel *m, double *stats){

  GetRNGstate();  /* R function enabling uniform RNG */
  
  WtShuffleEdges(heads,tails,weights,n_edges); /* Shuffle edgelist. */
  
  for (unsigned int termi=0; termi < m->n_terms; termi++)
    m->termarray[termi].dstats = m->workspace;
  
  /* Doing this one edge at a time saves a lot of toggles... */
  for(Edge e=0; e<n_edges; e++){
    WtModelTerm *mtp = m->termarray;
    double *statspos=stats;
    
    for (unsigned int termi=0; termi < m->n_terms; termi++, mtp++){
      if(!mtp->s_func){
        (*(mtp->d_func))(1, heads+e, tails+e, weights+e,
        mtp, nwp);  /* Call d_??? function */
        for (unsigned int i=0; i < mtp->nstats; i++,statspos++)
          *statspos += mtp->dstats[i];
      }else statspos += mtp->nstats;
    }
    
    WtSetEdge(heads[e],tails[e],weights[e],nwp);
  }
  
  WtModelTerm *mtp = m->termarray;
  double *dstats = m->workspace;
  double *statspos=stats;
  for (unsigned int termi=0; termi < m->n_terms; termi++, dstats+=mtp->nstats, mtp++ ){
    if(mtp->s_func){
      (*(mtp->s_func))(mtp, nwp);  /* Call s_??? function */
      for (unsigned int i=0; i < mtp->nstats; i++,statspos++)
        *statspos += mtp->dstats[i];
    }else statspos += mtp->nstats;
  }
  
  PutRNGstate();
}
