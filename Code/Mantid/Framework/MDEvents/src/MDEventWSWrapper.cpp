#include "MantidMDEvents/MDEventWSWrapper.h"

namespace Mantid
{
namespace MDEvents
{

/** internal helper function to create empty MDEventWorkspace with nd dimensions and set up inernal pointer to this workspace
  tempate parameter:
     * nd -- number of dimensions

 *@param  targ_dim_names -- size-nd vector of string containing names of nd dimensions of the wrapped workspace
 *@param  targ_dim_ID    -- size-nd vector of strings containing id of the wrapped workspace dimensions
 *@param  targ_dim_units -- size-nd vector of stirngs containing units of the wrapped workspace dimensions
 *
 *@param  dimMin         -- size-nd vector of min values of dimensions of the target workspace
 *@param  dimMax         -- size-nd vector of max values of dimensions of the target workspace
 *@param  numBins        -- size-nd vector of number of bins, each dimension is split on single level          */
template<size_t nd>
void  MDEventWSWrapper::createEmptyEventWS(const Strings &targ_dim_names,const Strings &targ_dim_ID,const Strings &targ_dim_units,
                                           const std::vector<double> &dimMin, const std::vector<double> &dimMax,const std::vector<size_t> &numBins)
{

  boost::shared_ptr<MDEvents::MDEventWorkspace<MDEvents::MDEvent<nd>,nd> > ws = 
    boost::shared_ptr<MDEvents::MDEventWorkspace<MDEvents::MDEvent<nd>, nd> >(new MDEvents::MDEventWorkspace<MDEvents::MDEvent<nd>, nd>());

  size_t nBins(10);
  // Give all the dimensions
  for (size_t d=0; d<nd; d++)
  {
    if(!numBins.empty()) nBins=numBins[d];

    Geometry::MDHistoDimension * dim = new Geometry::MDHistoDimension(targ_dim_names[d], targ_dim_ID[d], targ_dim_units[d], 
                                                                      coord_t(dimMin[d]), coord_t(dimMax[d]), nBins);
     ws->addDimension(Geometry::MDHistoDimension_sptr(dim));
  }
  ws->initialize();

  // Build up the box controller
  // bc = ws->getBoxController();
  // Build up the box controller, using the properties in BoxControllerSettingsAlgorithm 
  //     this->setBoxController(bc);
  // We always want the box to be split (it will reject bad ones)
  //ws->splitBox();
   m_Workspace = ws;
}
/// terminator for attemtpting initate 0 dimensions workspace, will throw. 
template<>
void  MDEventWSWrapper::createEmptyEventWS<0>(const Strings &,const Strings &,const Strings &,const std::vector<double> &, const std::vector<double> &,const std::vector<size_t> &)
{
  throw(std::invalid_argument("MDEventWSWrapper:createEmptyEventWS can not be initated with 0 dimensions"));
}

/** templated by number of dimesnions function to add multidimensional data to the workspace 
* it is  expected that all MD coordinates are within the ranges of MD defined workspace, so no checks are performed

   tempate parameter:
     * nd -- number of dimensions

*@param sigErr   -- pointer to the beginning of 2*data_size array containing signal and squared error
*@param runIndex -- pointer to the beginnign of data_size  containing run index
*@param detId    -- pointer to the beginning of dataSize array containing detector id-s
*@param Coord    -- pointer to the beginning of dataSize*nd array containig the coordinates od nd-dimensional events
*
*@param dataSize -- the length of the vector of MD events
*/
template<size_t nd>
void MDEventWSWrapper::add_MDData(float *sigErr,uint16_t *runIndex,uint32_t* detId,coord_t* Coord,size_t dataSize)const
{

  MDEvents::MDEventWorkspace<MDEvents::MDEvent<nd>,nd> *const pWs = dynamic_cast<MDEvents::MDEventWorkspace<MDEvents::MDEvent<nd>,nd> *>(m_Workspace.get());
  if(!pWs)throw(std::bad_cast());

  for(size_t i=0;i<dataSize;i++)
  {
    pWs->addEvent(MDEvents::MDEvent<nd>(*(sigErr+2*i),*(sigErr+2*i+1),*(runIndex+i),*(detId+i),(Coord+i*nd)));
  }

  // This splits up all the boxes according to split thresholds and sizes.
  //Kernel::ThreadScheduler * ts = new ThreadSchedulerFIFO();
  //ThreadPool tp(NULL);
  //pWs->splitAllIfNeeded(NULL);
  //tp.joinAll();        

}
/// the function used in template metaloop termination on 0 dimensions and to throw the error in attempt to add data to 0-dimension workspace
template<>
void MDEventWSWrapper::add_MDData<0>(float *,uint16_t *,uint32_t*,coord_t* ,size_t)const
{ 
  throw(std::invalid_argument(" class has not been initiated, can not add data to 0-dimensional workspace"));
}


/// helper function to refresh centroid on MDEventWorkspace with nd dimensions 
template<size_t nd>
void  MDEventWSWrapper::calc_Centroid(void)
{

  MDEvents::MDEventWorkspace<MDEvents::MDEvent<nd>,nd> *const pWs = dynamic_cast<MDEvents::MDEventWorkspace<MDEvents::MDEvent<nd>,nd> *>(this->m_Workspace.get());
  if(!pWs) throw(std::bad_cast());

  pWs->getBox()->refreshCentroid(NULL);
}
/// the function used in template metaloop termination on 0 dimensions and as the function which will throw the error on undefined MDWorkspaceWrapper  
template<>
void  MDEventWSWrapper::calc_Centroid<0>(void)
{
  throw(std::invalid_argument(" class has not been initiated"));
}


/**function returns the number of dimensions in current MDEvent workspace or throws if the workspace has not been defined */
size_t MDEventWSWrapper::nDimensions()const
{
  if(m_NDimensions==0)  throw(std::invalid_argument("The workspace has not been initated yet"));     
  return size_t(m_NDimensions);
}

/** function creates empty MD event workspace with given parameters (workspace factory) and stores internal pointer to this workspace for further usage.
*  IT ASLO SETS UP W-TRANSFORMATON. TODO: reconsile w-transfo with MD geometry. 
*
*@param WSD the class which describes an MD workspace
*
*@returns shared pointer to the created workspace
*/
API::IMDEventWorkspace_sptr MDEventWSWrapper::createEmptyMDWS(const MDWSDescription &WSD)
{

  if(WSD.nDimensions()<1||WSD.nDimensions()>MAX_N_DIM)
  {
    std::string ERR=" Number of requested MD dimensions: "+boost::lexical_cast<std::string>(WSD.nDimensions())+
      " exceeds maximal number of MD dimensions: "+boost::lexical_cast<std::string>((int)MAX_N_DIM)+" instantiated during compilation\n";
    throw(std::invalid_argument(ERR));
  }

  m_NDimensions = (int)WSD.nDimensions();
  // call the particular function, which creates the workspace with n_dimensions
  (this->*(wsCreator[m_NDimensions]))(WSD.getDimNames(),WSD.getDimIDs(),WSD.getDimUnits(),WSD.getDimMin(),WSD.getDimMax(),WSD.getNBins());

  // set up the matrix, which convert momentums from Q in orthogonal crystal coordinate system and units of Angstrom^-1 to hkl or orthogonal hkl or whatevert
  m_Workspace->setWTransf(WSD.m_Wtransf);
  return m_Workspace;
}

/// set up existing workspace pointer as internal pointer for the class to perform proper MD operations on this workspace
void MDEventWSWrapper::setMDWS(API::IMDEventWorkspace_sptr spWS)
{
  m_Workspace   = spWS;
  m_NDimensions = m_Workspace->getNumDims();
}

/** method adds the data to the workspace which was initiated before;
*@param sigErr   -- pointer to the beginning of 2*data_size array containing signal and squared error
*@param runIndex -- pointer to the beginnign of data_size  containing run index
*@param detId    -- pointer to the beginning of dataSize array containing detector id-s
*@param Coord    -- pointer to the beginning of dataSize*nd array containig the coordinates od nd-dimensional events   
*
*@param dataSize -- the length of the vector of MD events
*/
void  MDEventWSWrapper::addMDData(std::vector<float> &sigErr,std::vector<uint16_t> &runIndex,std::vector<uint32_t> &detId,std::vector<coord_t> &Coord,size_t dataSize)const
{

  if(dataSize==0)return;
  // perform the actual dimension-dependent addition 
  (this->*(mdEvSummator[m_NDimensions]))(&sigErr[0],&runIndex[0],&detId[0],&Coord[0],dataSize);

}


/** method should be called at the end of the algorithm, to let the workspace manager know that it has whole responsibility for the workspace
(As the algorithm is static, it will hold the pointer to the workspace otherwise, not allowing the WS manager to delete WS on request or when it finds this usefull)*/
void MDEventWSWrapper::releaseWorkspace()
{
  // decrease the sp count by one
  m_Workspace.reset();
  // mark the number of dimensions invalid;
  m_NDimensions=0;
}


// the class instantiated by compiler at compilation time and generates the map,  
// between the number of dimensions and the function, which process this number of dimensions
template<size_t i>
class LOOP{
public:
  static inline void EXEC(MDEventWSWrapper *pH)
  {
    LOOP< i-1 >::EXEC(pH);
    pH->wsCreator[i]    = &MDEventWSWrapper::createEmptyEventWS<i>;
    pH->mdEvSummator[i] = &MDEventWSWrapper::add_MDData<i>;
    pH->mdCalCentroid[i]= &MDEventWSWrapper::calc_Centroid<i>;

  }
};
// the class terminates the compitlation-time metaloop and sets up functions which process 0-dimension workspace operations
template<>
class LOOP<0>
{
public:
  static inline void EXEC(MDEventWSWrapper *pH)
  {           
    pH->wsCreator[0]    =&MDEventWSWrapper::createEmptyEventWS<0>;
    pH->mdEvSummator[0] =&MDEventWSWrapper::add_MDData<0>;
    pH->mdCalCentroid[0]=&MDEventWSWrapper::calc_Centroid<0>;
  }
};

/**constructor */
MDEventWSWrapper::MDEventWSWrapper():
m_NDimensions(0)
{
  wsCreator.resize(MAX_N_DIM+1);
  mdEvSummator.resize(MAX_N_DIM+1);
  mdCalCentroid.resize(MAX_N_DIM+1);
  LOOP<MAX_N_DIM>::EXEC(this);
}

} // endnamespace MDEvents
} // endnamespace Mantid