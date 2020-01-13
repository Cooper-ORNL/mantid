// Mantid Repository : https://github.com/mantidproject/mantid
//
// Copyright &copy; 2018 ISIS Rutherford Appleton Laboratory UKRI,
//     NScD Oak Ridge National Laboratory, European Spallation Source
//     & Institut Laue - Langevin
// SPDX - License - Identifier: GPL - 3.0 +
#ifndef MANTID_DATAHANDLING_3MF_H_
#define MANTID_DATAHANDLING_3MF_H_

#include "MantidDataHandling/LoadShape.h"
#include "lib3mf_implicit.hpp"
#include <boost/shared_ptr.hpp>
#include <memory>

namespace Mantid {

namespace Geometry {
class MeshObject;
}
namespace DataHandling {

/**

Class to load and save .3mf files
.3mf format is a 3D manufacturing format for storing mesh descriptions
of multi-component objects + metadata about the overall model (eg scale)
and the individual components. The class currently uses the Lib3MF library
to parse the files

*/

class DLLExport Mantid3MFFileIO : public LoadShape {
public:
  Mantid3MFFileIO() : LoadShape(ScaleUnits::undefined) {
    Lib3MF::PWrapper wrapper = Lib3MF::CWrapper::loadLibrary();
    model = wrapper->CreateModel();
  };
  void LoadFile(std::string filename);
  void readMeshObjects(
      std::vector<boost::shared_ptr<Geometry::MeshObject>> &meshObjects,
      boost::shared_ptr<Geometry::MeshObject> &sample);
  void writeMeshObjects(std::vector<const Geometry::MeshObject *> meshObjects,
                        const Geometry::MeshObject *sample,
                        DataHandling::ScaleUnits scale);
  /*void writeMeshObjects(
      std::vector<boost::shared_ptr<Geometry::MeshObject>> meshObjects,
      boost::shared_ptr<Geometry::MeshObject> &sample,
      DataHandling::ScaleUnits scale);*/
  void setMaterialOnObject(std::string objectName, std::string materialName,
                           int materialColor);
  void saveFile(std::string filename);

private:
  Lib3MF::PModel model;
  /*std::vector<uint32_t> m_triangle;
  std::vector<Kernel::V3D> m_vertices;*/
  void
  ShowComponentsObjectInformation(Lib3MF::PComponentsObject componentsObject);
  void ShowTransform(sLib3MFTransform transform, std::string indent);
  void ShowMetaDataInformation(Lib3MF::PMetaDataGroup metaDataGroup);
  boost::shared_ptr<Geometry::MeshObject>
  loadMeshObject(Lib3MF::PMeshObject meshObject,
                 sLib3MFTransform buildTransform);
  void writeMeshObject(const Geometry::MeshObject &meshObject,
                       std::string name);
  void AddBaseMaterial(std::string materialName, int materialColor,
                       int &resourceID, Lib3MF_uint32 &materialPropertyID);
  int generateRandomColor();
};
} // namespace DataHandling
} // namespace Mantid
#endif /* MANTID_DATAHANDLING_3MF_H_ */