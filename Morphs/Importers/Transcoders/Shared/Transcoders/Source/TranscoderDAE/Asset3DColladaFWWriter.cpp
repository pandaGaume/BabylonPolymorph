/********************************************************
*                                                       *
*   Copyright (C) Microsoft. All rights reserved.       *
*                                                       *
********************************************************/
#include "TranscodersPch.h"


#include <iostream>
#include <TranscoderDAE/Asset3DColladaFWWriter.h>
#include <TranscoderDAE/DAEGeometryConverter.h>
#include <TranscoderDAE/DAESceneConverter.h>

#include <Asset3D\Asset3D.h>

#include <COLLADAFWGeometry.h>
#include <COLLADAFWVisualScene.h>

#include <COLLADASaxFWLIError.h>

using namespace Babylon::Transcoder; 
using namespace Babylon::Framework;


Asset3DWriterContext::Asset3DWriterContext(IResourceServer* resourceServer, ICancellationTokenPtr cancellationToken) :
	m_asset3D(std::make_shared<Asset3D>()), 
	m_cancellationToken(cancellationToken), 
	m_resourceServer(resourceServer)
{
}

Asset3DWriterContext::~Asset3DWriterContext() {
}

Asset3DColladaFWWriter::Asset3DColladaFWWriter(IResourceServer* resourceServer, ICancellationTokenPtr cancellationToken) :
	m_context(resourceServer, cancellationToken)
{
}

Asset3DColladaFWWriter::~Asset3DColladaFWWriter() {
}

/** If this method returns true, the loader stops parsing immediately. If severity is nor CRITICAL
and this method returns false, the loader continues loading.*/
bool Asset3DColladaFWWriter::handleError(const COLLADASaxFWL::IError* error) {
	std::cout << "Error :" << (error ? error->getFullErrorMessage(): "") << "\r\n";
	return false;
}


/** This method will be called if an error in the loading process occurred and the loader cannot
continue to to load. The writer should undo all operations that have been performed.
@param errorMessage A message containing informations about the error that occurred.
*/
void Asset3DColladaFWWriter::cancel(const COLLADAFW::String& errorMessage) {
	std::cout << "CANCEL cause of " << errorMessage << "\r\n";
}

/** This is the method called. The writer hast to prepare to receive data.*/
void Asset3DColladaFWWriter::start() {
	std::cout << "START\r\n";
}

/** This method is called after the last write* method. No other methods will be called after this.*/
void Asset3DColladaFWWriter::finish() {

	Asset3DWriterContextPtr ctx = getContext();
	std::shared_ptr<Asset3D> asset = getAsset3D();

	/**
	 * some Collada file does NOT define visual scenes, then do NOT instantiate models.
	 * this appear to be a default behaviors of some transcoder. In this case, only save the model as
	 * direct child node of the Asset3D, without any transformation.
	 */
	if (!ctx->hasVisualScenes()) {
		/// bind geometries to the asset.
		if (ctx->hasGeometries()) {
			std::map<COLLADAFW::UniqueId, std::shared_ptr<Mesh>>& lib = ctx->getGeometryLibrary();
			std::map<COLLADAFW::UniqueId, std::shared_ptr<Mesh>>::iterator it = lib.begin();
			while (it != lib.end())
			{
				std::shared_ptr<Mesh> m = it->second;
				std::shared_ptr<SceneNode> n = asset->CreateChildNode();
				if (n) {
					n->SetMesh(m);
				}
				it++;
			}
		}
		return;
	}

	/// simply push all the nodes...
	std::map<COLLADAFW::UniqueId, std::shared_ptr<SceneNode>>& lib = ctx->getVisualSceneLibrary();
	std::map<COLLADAFW::UniqueId, std::shared_ptr<SceneNode>>::iterator it = lib.begin();
	while (it != lib.end()) {
		std::shared_ptr<SceneNode> childNode = (*it).second;
		asset->AddChildNode(childNode);
		it++;
	}
}

/** When this method is called, the writer must write the global document asset.
@return The writer should return true, if writing succeeded, false otherwise.*/
bool Asset3DColladaFWWriter::writeGlobalAsset(const COLLADAFW::FileInfo* asset) {

	Asset3DWriterContextPtr ctx = getContext();
	ctx->setUpAxisType(asset->getUpAxisType());

	return true;
}

/** When this method is called, the writer must write the scene.
@return The writer should return true, if writing succeeded, false otherwise.*/
bool Asset3DColladaFWWriter::writeScene(const COLLADAFW::Scene* scene) {
	std::cout << "writeScene\r\n";
	return true;
}

/** When this method is called, the writer must write the entire visual scene.
@return The writer should return true, if writing succeeded, false otherwise.*/
bool Asset3DColladaFWWriter::writeVisualScene(const COLLADAFW::VisualScene* visualScene) {
	std::cout << "writeVisualScene\r\n";
	DAEVirtualSceneConverter c(&m_context);
	std::shared_ptr<SceneNode> s = c.GetNode(visualScene);
	if (s) {
		m_context.getVisualSceneLibrary()[visualScene->getUniqueId()] = s;
	}
	return true;
}

/** When this method is called, the writer must handle all nodes contained in the
library nodes.
@return The writer should return true, if writing succeeded, false otherwise.*/
bool Asset3DColladaFWWriter::writeLibraryNodes(const COLLADAFW::LibraryNodes* libraryNodes) {
	std::cout << "writeLibraryNodes\r\n";
	return true;
}

/** When this method is called, the writer must write the geometry.
@return The writer should return true, if writing succeeded, false otherwise.*/
bool Asset3DColladaFWWriter::writeGeometry(const COLLADAFW::Geometry* geometry) {
	std::cout << "writeGeometry " << geometry->getName() << "\r\n";

	switch (geometry->getType()) {
		case COLLADAFW::Geometry::GEO_TYPE_MESH: {
			DAEMeshConverter c( &m_context);
			std::shared_ptr<Mesh> m = c.GetNode((COLLADAFW::Mesh*)geometry);
			if (m) {
				m_context.getGeometryLibrary()[geometry->getUniqueId()] = m;
			}
			break;
		}
		default: {
			return false;
		}
	}
	return true;
}

/** When this method is called, the writer must write the material.
@return The writer should return true, if writing succeeded, false otherwise.*/
bool Asset3DColladaFWWriter::writeMaterial(const COLLADAFW::Material* material) {
	std::cout << "writeMaterial\r\n";
	return true;
}
/** When this method is called, the writer must write the effect.
@return The writer should return true, if writing succeeded, false otherwise.*/
bool Asset3DColladaFWWriter::writeEffect(const COLLADAFW::Effect* effect) {
	std::cout << "writeEffect\r\n";
	return true;
}

/** When this method is called, the writer must write the camera.
@return The writer should return true, if writing succeeded, false otherwise.*/
bool Asset3DColladaFWWriter::writeCamera(const COLLADAFW::Camera* camera) {
	std::cout << "writeCamera\r\n";
	return true;
}

/** When this method is called, the writer must write the image.
@return The writer should return true, if writing succeeded, false otherwise.*/
bool Asset3DColladaFWWriter::writeImage(const COLLADAFW::Image* image) {
	std::cout << "writeImage\r\n";
	return true;
}

/** When this method is called, the writer must write the light.
@return The writer should return true, if writing succeeded, false otherwise.*/
bool Asset3DColladaFWWriter::writeLight(const COLLADAFW::Light* light) {
	std::cout << "writeLight\r\n";
	return true;
}

/** When this method is called, the writer must write the Animation.
@return The writer should return true, if writing succeeded, false otherwise.*/
bool Asset3DColladaFWWriter::writeAnimation(const COLLADAFW::Animation* animation) {
	std::cout << "writeAnimation\r\n";
	return true;
}

/** When this method is called, the writer must write the AnimationList.
@return The writer should return true, if writing succeeded, false otherwise.*/
bool Asset3DColladaFWWriter::writeAnimationList(const COLLADAFW::AnimationList* animationList) {
	std::cout << "writeAnimationList\r\n";
	return true;
}

/** When this method is called, the writer must write the AnimationClip.
@return The writer should return true, of writing succeeded, false otherwise.*/
bool Asset3DColladaFWWriter::writeAnimationClip(const COLLADAFW::AnimationClip* animationClip) {
	std::cout << "writeAnimationClip\r\n";
	return true;
}

/** When this method is called, the writer must write the skin controller data.
@return The writer should return true, if writing succeeded, false otherwise.*/
bool Asset3DColladaFWWriter::writeSkinControllerData(const COLLADAFW::SkinControllerData* skinControllerData) {
	std::cout << "writeSkinControllerData\r\n";
	return true;
}

/** When this method is called, the writer must write the controller.
@return The writer should return true, if writing succeeded, false otherwise.*/
bool Asset3DColladaFWWriter::writeController(const COLLADAFW::Controller* controller) {
	std::cout << "writeController\r\n";

	const COLLADAFW::UniqueId& src = controller->getSource();
	m_context.getSkinLibrary()[controller->getUniqueId()] = src;

	return true;
}

/** When this method is called, the writer must write the formulas. All the formulas of the entire
COLLADA file are contained in @a formulas.
@return The writer should return true, if writing succeeded, false otherwise.*/
bool Asset3DColladaFWWriter::writeFormulas(const COLLADAFW::Formulas* formulas) {
	std::cout << "writeFormulas\r\n";
	return true;
}

/** When this method is called, the writer must write the kinematics scene.
@return The writer should return true, if writing succeeded, false otherwise.*/
bool Asset3DColladaFWWriter::writeKinematicsScene(const COLLADAFW::KinematicsScene* kinematicsScene) {
	std::cout << "writeKinematicsScene\r\n";
	return true;
}
