#include "StdAfx.h"
#include <dbosnap2.h>
#include <dbobjptr2.h>
#include <dbindex.h>
#include <dbboiler.h>
#include <AcDbPointCloudEx.h>
#include <AcPointCloudExtractedCylinder.h>




int getCurrentViewportNumber(void)
{
	struct resbuf   rb;
	ads_getvar(_T("CVPORT"), &rb);
	return rb.resval.rint;
}

struct cylinderElements
{
	AcGeVector3d _axis;
	AcGePoint3d _origin;
	double _height;
	double _raidus;

};



void selectPointCloud(AcDbObjectId& pcId)
{
	ads_name entres;
	ads_point ptres;
	if (RTNORM != acedEntSel(_T(""), entres, ptres)) {
		pcId = AcDbObjectId::kNull;
		return;
	}
	if(!eOkVerify(acdbGetObjectId(pcId,entres))) {
		pcId = AcDbObjectId::kNull; return;
	}

	return;
}

extern "C" __declspec(dllexport) 
Acad::ErrorStatus getCylinder(AcDbObjectId pcId,  cylinderElements& cElements)
{
	Acad::ErrorStatus es;
	
	ads_point result;
	if (RTNORM != acedGetPoint(NULL, _T("Pick a point on cylinder like element"), result)) return Acad::eInvalidInput;
	AcGeMatrix3d matWCS2DCS = AcGeMatrix3d::kIdentity;
	//Get viewingMatrix
	AcGsView* pView = acgsGetCurrent3dAcGsView(getCurrentViewportNumber());
	matWCS2DCS = pView->viewingMatrix();

	AcDbDatabase *pDb = acdbHostApplicationServices()->workingDatabase();

	AcDbSmartObjectPointer<AcDbPointCloudEx> pPc(pcId, AcDb::kForRead);
	//Check if object is ready to read.
	if ((es = pPc.openStatus()) != Acad::eOk) return es;

	AcPointCloudExtractedCylinder extractedCyl;
	AcGePoint3d clickPoint = asPnt3d(result);
	if ((es = pPc->getCylinderAt(matWCS2DCS, clickPoint, extractedCyl)) != Acad::eOk) return es;
	//We are good now, get the elements of Extracted cylinder and put in our Datastructure
	cElements._axis = extractedCyl.getAxis();
	cElements._origin = extractedCyl.getOrigin();
	cElements._height = extractedCyl.getHeight();
	cElements._raidus = extractedCyl.getRadius();
	return es;
}

void testPEX()
{
	AcDbObjectId pcId = AcDbObjectId::kNull;
	selectPointCloud(pcId);
	if (!pcId.isNull())
	{
		cylinderElements cElements;
		Acad::ErrorStatus es = getCylinder(pcId, cElements);
		acedAlert(acadErrorStatusText(es));
	}
}

//**************************************************************
extern "C" __declspec(dllexport) AcRx::AppRetCode acrxEntryPoint(AcRx::AppMsgCode msg, void *pkt)
//**************************************************************
{
	switch(msg)
	{
		case AcRx::kInitAppMsg:
			acrxDynamicLinker->unlockApplication(pkt);
			acrxDynamicLinker->registerAppMDIAware(pkt);
			acedRegCmds->addCommand(_T("TestCmd"), _T("PEX"), _T("PEX"), ACRX_CMD_MODAL, testPEX);

			break;
		case AcRx::kUnloadAppMsg:
			/*unload commands*/
			acedRegCmds->removeGroup(_T("TestCmd"));

			break;
		default:
			break;
	}
	return AcRx::kRetOK;
}