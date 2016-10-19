#include "stdafx.h"
#include "DesignerView.h"
#include "SMoveWnd.h"
#include "CNewGuid.h"
#include "helper\SplitString.h"
#include "DlgSkinSelect.h"
#include "DlgStyleManage.h"
#include "core/SWnd.h"
#include "MainDlg.h"

BOOL SDesignerView::NewLayout(SStringT strResName, SStringT strPath)
{
	SStringT strShortPath = strPath.Mid(m_strProPath.GetLength() + 1);

	pugi::xml_node xmlNode = m_xmlDocUiRes.child(_T("resource")).child(_T("LAYOUT"));

	if (xmlNode)
	{
		pugi::xml_node Child = xmlNode.append_child(_T("file"));
		Child.append_attribute(_T("name")).set_value(strResName);
		Child.append_attribute(_T("path")).set_value(strShortPath);

		m_xmlDocUiRes.save_file(m_strUIResFile);

	}

	return TRUE;

}


SDesignerView::SDesignerView(SHostDialog *pMainHost, SWindow *pContainer, STreeCtrl *pTreeXmlStruct)
{
	m_nState = 0;
	m_pMoveWndRoot = NULL;
	m_pRealWndRoot = NULL;
	m_pContainer = pContainer;
	m_pMainHost = pMainHost;
	m_treeXmlStruct = pTreeXmlStruct;

	m_treeXmlStruct->GetEventSet()->subscribeEvent(EVT_TC_SELCHANGED,Subscriber(&SDesignerView::OnTCSelChanged,this));


	pugi::xml_document doc;
	pugi::xml_parse_result result = doc.load_file(_T("Config\\Ctrl.xml"));
	if (!result)
	{
		Debug(_T("����Ctrl.xmlʧ��"));
		return;
	}

	pugi::xml_node node = doc.child(_T("root")).child(_T("�����ؼ�")).first_child();
	while (node)
	{
		m_lstContainerCtrl.AddTail(node.name());
		node = node.next_sibling();
	}

	SPOSITION pos = m_lstContainerCtrl.GetHeadPosition();
	while (pos)
	{
		SStringT strTemp = m_lstContainerCtrl.GetNext(pos);

	}
	m_privateStylePool.Attach(new SStylePool);
	m_privateSkinPool.Attach(new SSkinPool);
}

BOOL SDesignerView::OpenProject(SStringT strFileName)
{
	m_xmlDocUiRes.load_file(strFileName);



	m_strUIResFile = strFileName;
	TCHAR *s = strFileName.GetBuffer(strFileName.GetLength());

	PathRemoveFileSpec(s);
	SStringT strTemp(s);
	m_strProPath = strTemp;
	CAutoRefPtr<IResProvider>   pResProvider;

	CreateResProvider(RES_FILE, (IObjRef**)&pResProvider);
	if (!pResProvider->Init((LPARAM)s, 0))
	{
		Debug(_T("CreateResProviderʧ��"));
		return FALSE;
	}
	SApplication::getSingletonPtr()->AddResProvider(pResProvider);

	return TRUE;
}


BOOL SDesignerView::InsertLayoutToMap(SStringT strFileName)
{
	SStringT FullFileName = m_strProPath + _T("\\") + strFileName;

	pugi::xml_document *xmlDoc1 = new pugi::xml_document();

	//if(!xmlDoc1->load_file(FullFileName,pugi::parse_default,pugi::encoding_utf8))
	if(!xmlDoc1->load_file(FullFileName))
		return FALSE;

	m_mapLayoutFile[strFileName] = xmlDoc1->first_child();

	m_mapLayoutFile1[strFileName] = xmlDoc1;  

	RenameChildeWnd(xmlDoc1->first_child());
	return TRUE;
}

BOOL SDesignerView::LoadLayout(SStringT strFileName)
{


	pugi::xml_node xmlroot;
	pugi::xml_node xmlnode;

	BOOL bIsInclude = FALSE;

	m_strCurFile = strFileName;

	SMap<SStringT, pugi::xml_node>::CPair *p = m_mapLayoutFile.Lookup(strFileName);
	xmlroot = p->m_value;

	m_CurrentLayoutNode = xmlroot;


	if (xmlroot == NULL)
	{
		return TRUE;
	}

	if (S_CW2T(xmlroot.name()) != _T("SOUI"))
	{
		//include�ļ�
		xmlnode = xmlroot;
		bIsInclude = TRUE;
	}
	else
	{
		//����˽��Ƥ��
		if (m_privateStylePool->GetCount())
		{
			m_privateStylePool->RemoveAll();
			GETSTYLEPOOLMGR->PopStylePool(m_privateStylePool);
		}
		BOOL ret=m_privateStylePool->Init(xmlroot.child(L"style"));
		if (ret)
		{
			GETSTYLEPOOLMGR->PushStylePool(m_privateStylePool);
		}
		if (m_privateSkinPool->GetCount())
		{
			m_privateSkinPool->RemoveAll();
			GETSKINPOOLMGR->PopSkinPool(m_privateSkinPool);
		}
		int skincount=m_privateSkinPool->LoadSkins(xmlroot.child(L"skin"));//��xmlNode�Ӽ���˽��skin
		if (skincount)
		{
			GETSKINPOOLMGR->PushSkinPool(m_privateSkinPool);
		}

		xmlnode=xmlroot.child(L"root", false);
	}
	if(!xmlnode) return FALSE;



	//����ط�����Ҫ�޸ģ���������������������������������������������������������������������������

	SWindow *pChild = m_pContainer->GetWindow(GSW_FIRSTCHILD);
	while(pChild)
	{
		SWindow *pNext = pChild->GetWindow(GSW_NEXTSIBLING);
		pChild->DestroyWindow();
		pChild = pNext;
	}


	SStringW s1, s2;
	
	if (!bIsInclude)
	{
		pugi::xml_attribute attr = m_CurrentLayoutNode.first_attribute();
		while (attr)
		{
			s1.Format(L" %s=\"%s\" ", attr.name(), attr.value());
			s2 = s2 + s1;

			attr = attr.next_attribute();
		}

		attr = xmlnode.first_attribute();
		while (attr)
		{
			s1.Format(L" %s=\"%s\" ", attr.name(), attr.value());
			s2 = s2 + s1;

			attr = attr.next_attribute();
		}

		s2 = L"<window pos=\"20,20\" " +  s2 + L"></window>";

	}else
	{
    	s2 = L"<window pos=\"20,20,-20,-20\" colorBkgnd=\"#d0d0d0\"></window>";
	}

	//wchar_t *s = L"<window pos=\"20,20,@500,@500\" colorBkgnd=\"#d0d0d0\"></window>";
	wchar_t *s3 = L"<movewnd pos=\"20,20,@500,@500\" ></movewnd>";

	////�������ִ��ڵĸ�����

	m_pRealWndRoot = m_pContainer->CreateChildren(s2);
	m_pMoveWndRoot = (SMoveWnd *)m_pContainer->CreateChildren(s3);
	m_pMoveWndRoot->m_pRealWnd = m_pRealWndRoot;

	m_mapMoveRealWnd.RemoveAll();
	m_mapMoveRealWnd[m_pMoveWndRoot->m_pRealWnd] = m_pMoveWndRoot;


	m_pMoveWndRoot->m_Desiner = this;	




	if (!m_pRealWndRoot->CreateChildren(xmlnode))
	{
		return FALSE;
	}

	CreateAllChildWnd(m_pRealWndRoot, m_pMoveWndRoot);

	m_nState = 0;
	GetMoveWndRoot()->Click(0, CPoint(0,0));

	m_treeXmlStruct->RemoveAllItems();
	InitXMLStruct(m_CurrentLayoutNode, STVI_ROOT);
	return TRUE;
}

void SDesignerView::CreateAllChildWnd(SWindow *pRealWnd, SMoveWnd *pMoveWnd)
{
	////�õ���һ���Ӵ���
	SWindow *pSibReal = pRealWnd->GetWindow(GSW_FIRSTCHILD);
	for (; pSibReal; pSibReal = pSibReal->GetWindow(GSW_NEXTSIBLING))
	{
		wchar_t *s1 = L"<movewnd pos=\"0,0,@100,@100\" ></movewnd>";

		//�������ִ��ڵĸ�����
		SMoveWnd *pSibMove = (SMoveWnd *)pMoveWnd->CreateChildren(s1);
		pSibMove->m_pRealWnd = pSibReal;
		pSibMove->SetVisible(pSibReal->IsVisible());

		m_mapMoveRealWnd[pSibReal] = pSibMove;

		pSibMove->m_Desiner = this;	

		CreateAllChildWnd(pSibReal, pSibMove);


	}
}

void SDesignerView::UpdateAllLayout(SMoveWnd* pMoveWndRoot)
{
	int b = pMoveWndRoot->GetChildrenCount();
	////�õ���һ���Ӵ���
	SMoveWnd *pSibMove = (SMoveWnd*)pMoveWndRoot->GetWindow(GSW_FIRSTCHILD);
	for (; pSibMove; pSibMove = (SMoveWnd*)pSibMove->GetWindow(GSW_NEXTSIBLING))
	{

		//�����������ڵĲ���
		SwndLayout *pMoveWndLayout = pSibMove->GetLayout();
		SwndLayout *pRealWndLayout = pSibMove->m_pRealWnd->GetLayout();

		pMoveWndLayout->uPositionType = pRealWndLayout->uPositionType;
		pMoveWndLayout->nCount = pRealWndLayout->nCount;

		for (int i = 0; i < 4; i++)
		{
			pMoveWndLayout->pos[i].cMinus = pRealWndLayout->pos[i].cMinus;
			pMoveWndLayout->pos[i].nPos = pRealWndLayout->pos[i].nPos;
			pMoveWndLayout->pos[i].nRefID = pRealWndLayout->pos[i].nRefID;
			pMoveWndLayout->pos[i].pit = pRealWndLayout->pos[i].pit;
		}
		pMoveWndLayout->fOffsetX = pRealWndLayout->fOffsetX;
		pMoveWndLayout->fOffsetY = pRealWndLayout->fOffsetY;
		pMoveWndLayout->SetWidth(pRealWndLayout->GetSpecifySize(PD_X));
		pMoveWndLayout->SetHeight(pRealWndLayout->GetSpecifySize(PD_Y));

		UpdateAllLayout(pSibMove);


	}
}


SDesignerView::~SDesignerView()
{
	m_pMoveWndRoot->m_pRealWnd->DestroyWindow();
	m_pMoveWndRoot->DestroyWindow();
	GETSTYLEPOOLMGR->PopStylePool(m_privateStylePool);
	GETSKINPOOLMGR->PopSkinPool(m_privateSkinPool);
}

BOOL SDesignerView::SaveAll()
{
	SMap<SStringT, pugi::xml_document*>::CPair *p;
	SStringT strFileName;
	SStringT FullFileName;
	pugi::xml_document *doc;
	pugi::xml_document DocSave;


	SPOSITION pos = m_mapLayoutFile1.GetStartPosition();

	while (pos)
	{
		p = m_mapLayoutFile1.GetNext(pos);
		strFileName = p->m_key;
		doc = p->m_value;

		pugi::xml_writer_buff writer;
		doc->print(writer,L"\t",pugi::format_default,pugi::encoding_utf16);
		SStringW *strxmlWnd= new SStringW(writer.buffer(),writer.size());

		if(DocSave.load_buffer(*strxmlWnd,wcslen(*strxmlWnd)*sizeof(wchar_t),pugi::parse_default,pugi::encoding_utf16)) 
		{
			pugi::xml_node NodeSave = DocSave.first_child();
			RemoveWndName(NodeSave);

			FullFileName = m_strProPath + _T("\\") + strFileName;
			DocSave.save_file(FullFileName);
		}else
		{
			Debug(_T("�����ļ�ʧ�ܣ�") + FullFileName);
		}
		delete strxmlWnd;

	}

	m_xmlDocUiRes.save_file(m_strUIResFile);
	Debug(_T("����ɹ�"));


	return TRUE;
}

//���浱ǰ�򿪵Ĳ����ļ�
BOOL SDesignerView::SaveLayoutFile(SStringT strFile)
{
	SStringT strFileName;
	SStringT FullFileName;

	SMap<SStringT, pugi::xml_document*>::CPair *p = m_mapLayoutFile1.Lookup(strFile);
	strFileName = p->m_key;
	pugi::xml_document *doc = p->m_value;

	pugi::xml_writer_buff writer;
	doc->print(writer,L"\t",pugi::format_default,pugi::encoding_utf16);
	SStringW *strxmlWnd= new SStringW(writer.buffer(),writer.size());


	pugi::xml_document DocSave;
	if(DocSave.load_buffer(*strxmlWnd,wcslen(*strxmlWnd)*sizeof(wchar_t),pugi::parse_default,pugi::encoding_utf16))
	{
		pugi::xml_node NodeSave = DocSave.first_child();
	    RemoveWndName(NodeSave);

		FullFileName = m_strProPath + _T("\\") + strFileName;
		DocSave.save_file(FullFileName);
	}else
	{
		Debug(_T("����ʧ��"));
	}

	delete strxmlWnd;

	m_xmlDocUiRes.save_file(m_strUIResFile);

	return TRUE;
}

//�رյ�ǰ�򿪵Ĳ����ļ�
BOOL SDesignerView::CloseLayoutFile()
{
	return TRUE;
}

//��������
SMoveWnd* SDesignerView::CreateWnd(SWindow *pContainer,LPCWSTR pszXml)
{


	SWindow *pChild = pContainer->CreateChildren(pszXml);
	((SMoveWnd*)pChild)->m_Desiner = this;
	m_CurSelCtrl = (SMoveWnd*)pChild;
	return (SMoveWnd*)pChild;
}

void SDesignerView::RenameWnd(pugi::xml_node xmlNode, BOOL force)
{
	pugi::xml_attribute xmlAttr = xmlNode.attribute(L"name", false);
	pugi::xml_attribute xmlAttr1 = xmlNode.attribute(L"uidesiner_name", false);

	SStringT strNameValue;

	if (force)
	{
		if (!xmlAttr)
		{
			strNameValue = CNewGuid::Get();
			xmlNode.append_attribute(L"name").set_value(strNameValue);
		}else
		{
			strNameValue = CNewGuid::Get();
			xmlAttr.set_value(strNameValue);
		}

		if (!xmlAttr1)
		{
			xmlNode.append_attribute(L"uidesiner_name").set_value(strNameValue);
		}else
		{
			xmlAttr1.set_value(strNameValue);
		}
	}
	else
	{
		if (!xmlAttr)
		{
			strNameValue = CNewGuid::Get();
			xmlNode.append_attribute(L"name").set_value(strNameValue);
			xmlNode.append_attribute(L"uidesiner_name").set_value(strNameValue);
		}else
		{
			strNameValue = xmlAttr.value();

			if (!xmlAttr1)
			{
				xmlNode.append_attribute(L"uidesiner_name").set_value(strNameValue);
			}
			else
			{
				xmlAttr1.set_value(strNameValue);
			}

			xmlAttr.set_value(CNewGuid::Get());
		}
	}
}



void SDesignerView::RemoveWndName(pugi::xml_node xmlNode)
{
	pugi::xml_node NodeChild = xmlNode.first_child();

	pugi::xml_attribute attr, attr1;
	pugi::xml_document doc;

	while (NodeChild)
	{  
		attr = NodeChild.attribute(L"uidesiner_name",false);
		attr1 = NodeChild.attribute(L"name", false);

		SMap<SStringT, SStringT>::CPair *p = m_mapInclude.Lookup(attr1.value());
		

		if (p)
		{//�������ؼ���include

			if(!doc.load_buffer(p->m_value,wcslen(p->m_value)*sizeof(wchar_t),pugi::parse_default,pugi::encoding_utf16)) 
			{	
				Debug(_T("RemoveWndName������"));
			}
			else
			{
				pugi::xml_node nodeNew;
				nodeNew = NodeChild.parent().insert_copy_after(doc.first_child(), NodeChild);
				NodeChild.parent().remove_child(NodeChild);
				NodeChild = nodeNew.next_sibling();
			}

		}else
		{
			if (attr)
			{
				SStringT str;
				str = attr.value();
				if (0 == str.CompareNoCase(attr1.value()))
				{
					NodeChild.remove_attribute(L"uidesiner_name");
					NodeChild.remove_attribute(L"name");
				}
				else
				{
					attr1.set_value(str);
					NodeChild.remove_attribute(L"uidesiner_name");
				}

			}

			RemoveWndName(NodeChild);

			NodeChild = NodeChild.next_sibling();
		}
	}

}

void SDesignerView::RenameChildeWnd(pugi::xml_node xmlNode, BOOL force)
{

	pugi::xml_node NodeChild = xmlNode.first_child();
	

	while (NodeChild)
	{  
		if (_wcsicmp(NodeChild.name(), L"style") == 0 || _wcsicmp(NodeChild.name(), L"skin") == 0)
		{
			NodeChild = NodeChild.next_sibling();
			continue;
		}
		//�滻Include
		if(_wcsicmp(NodeChild.name(),L"include")==0 && NodeChild.attribute(L"src"))
		{
			SStringT strInclude = NodeToStr(NodeChild);
			NodeChild.set_name(L"window");
			NodeChild.remove_attribute(L"src");
			NodeChild.append_attribute(L"pos").set_value(L"10,10,-10,-10");
			NodeChild.append_attribute(L"colorBkgnd").set_value(L"RGB(191,141,255)");
			NodeChild.append_attribute(L"colorBkgnd").set_value(L"RGB(191,141,255)");
			NodeChild.text().set(strInclude);

			RenameWnd(NodeChild);
			m_mapInclude[NodeChild.attribute(L"name").value()] = strInclude;
			
			NodeChild = NodeChild.next_sibling();


			continue;

		}


		////�ж�NodeChild.name()���͵Ŀؼ��Ƿ�ע��
  //      if (!static_cast<SWindowFactoryMgr*>(SApplication::getSingletonPtr())->HasKey(NodeChild.name()))
		//{
		//	RenameChildeWnd(NodeChild);

		//	NodeChild = NodeChild.next_sibling();
		//	continue;
		//}

		//��<button name = "XXX"/> �޸�Ϊ
		//  <button name = "sdfasf_asdf_sdfmij_dfwef" uidesiner_name = "XXX"/> 
		RenameWnd(NodeChild);


		RenameChildeWnd(NodeChild);

		NodeChild = NodeChild.next_sibling();
	}

}

void SDesignerView::RenameAllLayoutWnd()
{
	SPOSITION pos = m_mapLayoutFile.GetStartPosition();
	while (pos)
	{
		SMap<SStringT, pugi::xml_node>::CPair *p = m_mapLayoutFile.GetNext(pos);
		RenameChildeWnd(p->m_value);
	}	
}

pugi::xml_node SDesignerView::FindNodeByAttr(pugi::xml_node NodeRoot, SStringT attrName, SStringT attrValue)
{
	pugi::xml_node NodeChild = NodeRoot.first_child();

	pugi::xml_attribute attr;
	pugi::xml_node NodeResult;

	while (NodeChild)
	{   

		
		attr = NodeChild.attribute(attrName, false);
		if (attr)
		{
			if (0 == attrValue.CompareNoCase(attr.value()))
			{
				return NodeChild;
			}
		}

		NodeResult = FindNodeByAttr(NodeChild, attrName, attrValue);
		if (NodeResult)
		{
			return NodeResult;
		}

		NodeChild = NodeChild.next_sibling();
	}

	return NodeResult;
}


void SDesignerView::Debug(pugi::xml_node xmlNode)
{
	pugi::xml_writer_buff writer;
	xmlNode.print(writer,L"\t",pugi::format_default,pugi::encoding_utf16);
	SStringW *strDebug= new SStringW(writer.buffer(),writer.size());
	SMessageBox(NULL, *strDebug, _T(""), MB_OK);
	delete strDebug;
}

void SDesignerView::Debug(SStringT str)
{
	SMessageBox(NULL, str, _T(""), MB_OK);

}

SStringT SDesignerView::Debug1(pugi::xml_node xmlNode)
{
	pugi::xml_writer_buff writer;
	xmlNode.print(writer,L"\t",pugi::format_default,pugi::encoding_utf16);
	SStringW *strDebug= new SStringW(writer.buffer(),writer.size());
	SStringT strtemp = *strDebug;
	delete strDebug;
	return strtemp;
}

SStringT SDesignerView::NodeToStr(pugi::xml_node xmlNode)
{
	pugi::xml_writer_buff writer;
	xmlNode.print(writer,L"\t",pugi::format_default,pugi::encoding_utf16);
	SStringW *strDebug= new SStringW(writer.buffer(),writer.size());
    SStringT str(*strDebug);
	delete strDebug;
	return str;
}


void SDesignerView::SetCurrentCtrl(pugi::xml_node xmlNode, SMoveWnd *pWnd)
{
	//m_xmlDoc.remove_child(m_xmlDoc.first_child());
 //   m_xmlDoc.append_copy(xmlNode);
	//m_xmlNode = m_xmlDoc.first_child();
	m_xmlNode= xmlNode;
	m_CurSelCtrl = pWnd;
	
	m_treeXmlStruct->GetEventSet()->unsubscribeEvent(EVT_TC_SELCHANGED,Subscriber(&SDesignerView::OnTCSelChanged,this));
    GoToXmlStructItem(m_CurSelCtrl->m_pRealWnd->GetName(), m_treeXmlStruct->GetRootItem());
	m_treeXmlStruct->GetEventSet()->subscribeEvent(EVT_TC_SELCHANGED,Subscriber(&SDesignerView::OnTCSelChanged,this));
}


void SDesignerView::UpdatePosToXmlNode(SWindow *pRealWnd, SMoveWnd* pMoveWnd)
{
	SwndLayout *pLayout = pRealWnd->GetLayout();

	pugi::xml_node xmlNode = FindNodeByAttr(m_CurrentLayoutNode, L"name", pRealWnd->GetName());
	if(!xmlNode)
	{
		return;
	}

	pugi::xml_attribute attrPos, attrSize, attrOffset, attrPos2type;

	SStringW strTemp;
	attrPos = xmlNode.attribute(L"pos");
	attrSize = xmlNode.attribute(L"size");
	attrOffset = xmlNode.attribute(L"offset");
	attrPos2type = xmlNode.attribute(L"pos2type");

	if (attrSize)
	{
		strTemp.Format(_T("%d, %d"), pLayout->GetSpecifySize(PD_X), pLayout->GetSpecifySize(PD_Y));
		attrSize.set_value(strTemp);
	}

	if (attrPos2type)
	{
		//ɾ��Pos2Type,�ĳ�attrOffset
		xmlNode.remove_attribute(L"pos2type");
		if (!attrOffset)
		{
			xmlNode.append_attribute(L"offset");
			attrOffset = xmlNode.attribute(L"offset");
		}
	}

	if (attrOffset)
	{
		strTemp.Format(_T("%g, %g"), pLayout->fOffsetX, pLayout->fOffsetY);
		attrOffset.set_value(strTemp);
	}

	if (attrPos)
	{   
		if (pLayout->nCount == 2)
		{
			strTemp = GetPosFromLayout(pLayout, 0) + _T(",");
			strTemp = strTemp + GetPosFromLayout(pLayout, 1);
			attrPos.set_value(strTemp);
		}else if (pLayout->nCount == 4)
		{
			strTemp = GetPosFromLayout(pLayout, 0) + _T(",");
			strTemp = strTemp + GetPosFromLayout(pLayout, 1) + _T(",");
			strTemp = strTemp + GetPosFromLayout(pLayout, 2) + _T(",");
			strTemp = strTemp + GetPosFromLayout(pLayout, 3);
			attrPos.set_value(strTemp);
		}
	}

	SetCurrentCtrl(xmlNode, pMoveWnd);

}

SStringW SDesignerView::GetPosFromLayout(SwndLayout *pLayout, INT nPosIndex)
{
	SStringW strPos;

	switch (pLayout->pos[nPosIndex].pit)
	{
	case PIT_NULL: 
		strPos = L"";        //��Ч����
		break;
	case PIT_NORMAL:        //ê������
		strPos = L"";
		break;
	case PIT_CENTER:        //�ο����������ĵ�,��"|"��ʼ
		strPos = L"|";
		break;
	case PIT_PERCENT:       //ָ���ڸ�����������еİٷֱ�,��"%"��ʼ
		strPos = L"%";
		break;
	case PIT_PREV_NEAR:     //�ο�ǰһ���ֵܴ������Լ����ı�,��"["��ʼ
		strPos = L"[";
		break;
	case PIT_NEXT_NEAR:     //�ο���һ���ֵܴ������Լ����ı�,��"]"��ʼ
		strPos = L"]";
		break;
	case PIT_PREV_FAR:     //�ο�ǰһ���ֵܴ������Լ�Զ�ı�,��"{"��ʼ
		strPos = L"{";
		break;
	case PIT_NEXT_FAR:      //�ο���һ���ֵܴ������Լ�Զ�ı�,��"}"��ʼ
		strPos = L"}";
		break;
	case PIT_SIZE:          //ָ�����ڵĿ����߸�,��"@"��ʼ
		strPos = L"@";
		break;
	case PIT_SIB_LEFT:      //�ֵܽ���left,����X
		if (0 == nPosIndex)
		{
			strPos = strPos.Format(L"sib.left@%d:", pLayout->pos[nPosIndex].nRefID);
		}
		else
		{
			strPos = strPos.Format(L"sib.top@%d:", pLayout->pos[nPosIndex].nRefID);
		}

		break;

		//case PIT_SIB_TOP:      //�ֵܽ���top����left��ͬ������Y
		//	break;

	case PIT_SIB_RIGHT:      //�ֵܽ���right,����X 
		if (2 == nPosIndex)
		{
			strPos = strPos.Format(L"sib.right@%d:", pLayout->pos[nPosIndex].nRefID);
		}
		else
		{
			strPos = strPos.Format(L"sib.bottom@%d:", pLayout->pos[nPosIndex].nRefID);
		}

		break;

		//case PIT_SIB_BOTTOM:      //�ֵܽ���bottom,��right��ͬ,����Y 
		//	break;

	default:
		break;
	}

	if (pLayout->pos[nPosIndex].cMinus == -1)
	{
		strPos = strPos + L"-";
	}
	SStringW strTemp;
	int n = (int)pLayout->pos[nPosIndex].nPos;
	strTemp.Format(L"%d", n);
	strPos = strPos + strTemp;
	return strPos;
}



void SDesignerView::InitProperty(SWindow *pPropertyContainer)   //��ʼ�������б�
{
	m_pPropertyContainer = pPropertyContainer;
	/*

	<ͨ����ʽ>
		<id style="proptext" name ="����ID(id)" value="" />
		<name style="proptext" name ="��������(name)" value="" />	
		<skin style="proptext" name ="Ƥ��(skin)" value="" />		
	</ͨ����ʽ>

	<Button>
		<���� name="����">
		<id/>
		<name/>
		<skin/>
		<pos/>
		<size/>
		<offset/>
		</����>

		<���� name="��չ">
		<accel style="proptext" name ="��ݼ�(accel)" value="ctrl+alt+f9" />
		<animate style="propoption" name ="����(animate)" value="0" options="��(0)|��(1)"/>	
		</����>

	</Button>
	*/

	SStringW s = L"<propgrid name=\"NAME_UIDESIGNER_PROPGRID_MAIN\" pos=\"0,0,-4,-4\" switchSkin=\"SKIN_UIDESIGNER_PROPGRID_SWITCH\"                      \
		nameWidth=\"100\" orderType=\"group\"   itemHeight=\"30\"   ColorGroup=\"RGB(96,112,138)\"                                          \
		ColorItemSel=\"rgb(234,128,16)\" colorItemSelText=\"#FF0000\" EditBkgndColor=\"rgb(87,104,132)\"                                    \
		autoWordSel=\"1\"> <cmdbtnstyle skin=\"_skin.sys.btn.normal\">...</cmdbtnstyle> </propgrid>";


	pugi::xml_document m_xmlDocProperty;

	pugi::xml_parse_result result = m_xmlDocProperty.load_file(L"Config\\property.xml");
	if (!result)
	{
			Debug(_T("InitPropertyʧ��"));
	}

	

	pugi::xml_node NodeCom = m_xmlDocProperty.child(L"root").child(L"ͨ����ʽ");

	pugi::xml_node NodeCtrlList = m_xmlDocProperty.child(L"root").child(L"�����б�");


	//hostwnd�ڵ㴦��
	pugi::xml_node NodeCtrl = NodeCtrlList.child(_T("hostwnd")).first_child();
	m_lstSouiProperty.RemoveAll();
	pugi::xml_node NodeCtrlChild = NodeCtrl.first_child();
	while (NodeCtrlChild)
	{
		m_lstSouiProperty.AddTail(NodeCtrlChild.name());
		NodeCtrlChild = NodeCtrlChild.next_sibling();
	}

	NodeCtrl = NodeCtrl.next_sibling();
	m_lstRootProperty.RemoveAll();
	NodeCtrlChild = NodeCtrl.first_child();
	while (NodeCtrlChild)
	{
		m_lstRootProperty.AddTail(NodeCtrlChild.name());
		NodeCtrlChild = NodeCtrlChild.next_sibling();
	}



	NodeCtrl = NodeCtrlList.first_child();  //NodeCtrl = Button�ڵ�
	while (NodeCtrl)
	{
		InitCtrlProperty(NodeCom, NodeCtrl);

		SStringT strName = NodeCtrl.name();
		NodeCtrl.set_name(L"groups");


		pugi::xml_document *doc = new pugi::xml_document();

		if(!doc->load_buffer(s,wcslen(s)*sizeof(wchar_t),pugi::parse_default,pugi::encoding_utf16))
		{
			Debug(_T("InitPropertyʧ��1"));
		}

		doc->child(L"propgrid").append_copy(NodeCtrl);

		m_mapCtrlProperty[strName.MakeLower()] = doc;


		NodeCtrl = NodeCtrl.next_sibling();
	}






}


void SDesignerView::InitCtrlProperty(pugi::xml_node NodeCom, pugi::xml_node NodeCtrl)
{
	/*
	<ͨ����ʽ>
		<id style="proptext" name ="����ID(id)" value="" />
		<name style="proptext" name ="��������(name)" value="" />	
		<skin style="proptext" name ="Ƥ��(skin)" value="" />		
	</ͨ����ʽ>

	<Button>
		<���� name="����">
			<id/>
			<name/>
			<skin/>
			<pos/>
			<size/>
			<offset/>
		</����>

		<���� name="��չ">
			<accel style="proptext" name ="��ݼ�(accel)" value="ctrl+alt+f9" />
			<animate style="propoption" name ="����(animate)" value="0" options="��(0)|��(1)"/>	
		</����>

	</Button>

	<propgroup name="group1" description="desc of group1">
	<proptext name="text1.1" value="value 1.1">

	*/


	pugi::xml_node NodeChild = NodeCtrl.first_child();

	while (NodeChild)
	{
		if (_wcsicmp(NodeChild.name(), L"����") == 0)
		{
			NodeChild.set_name(L"propgroup");
			InitCtrlProperty(NodeCom, NodeChild);
		}else
		{
			if (!NodeChild.attribute(L"style"))
			{

				SStringT strName = NodeChild.name();
				pugi::xml_node N = NodeCom.child(strName);
				pugi::xml_node NodeNew;

				NodeNew = NodeChild.parent().insert_copy_before(N, NodeChild);
				NodeChild.parent().remove_child(NodeChild);

				NodeChild = NodeNew;
			}

			NodeChild.append_attribute(L"name2").set_value(NodeChild.name());

			NodeChild.set_name(NodeChild.attribute(L"style").value());
			NodeChild.remove_attribute(L"style");
		}

		NodeChild = NodeChild.next_sibling();
	}

}



void SDesignerView::CreatePropGrid(SStringT strCtrlType)
{
	//if (m_strCurrentCtrlType.CompareNoCase(strCtrlType) != 0)
	//{
			m_pPropgrid = (SPropertyGrid *)m_pPropertyContainer->GetWindow(GSW_FIRSTCHILD);
			if (m_pPropgrid)
			{
				//����һ���Ӱ���Ҫ�������Ų���������Ϊ�㰴ť��ʱ��PropGrid��û��ʧȥ���㣬
				//û��ִ��Killfocus��������Edit,��DestroyChild�Ժ�PropGrid�Ѿ������ˣ���ʱ����ִ��PropGrid��edit��killfocus�ᱨ��		
				
				m_pPropgrid->SetFocus();


				m_pPropertyContainer->DestroyChild(m_pPropgrid);
				m_pPropgrid = NULL;
			}

			SStringT strTemp;
			strTemp = m_CurrentLayoutNode.name();

			if (strCtrlType.CompareNoCase(_T("hostwnd")) == 0 && strTemp.CompareNoCase(_T("SOUI")) !=0 )
			{   //include �ļ�
				return;
			}

			SMap<SStringT, pugi::xml_document*>::CPair *p = m_mapCtrlProperty.Lookup(strCtrlType.MakeLower());
			if (p)
			{
			    pugi::xml_document *tempDoc = p->m_value;

				m_pPropertyContainer->CreateChildren(tempDoc->root());

				m_pPropgrid = (SPropertyGrid *)m_pPropertyContainer->GetWindow(GSW_FIRSTCHILD);
				m_pPropertyContainer->Invalidate();
				

				m_pPropgrid-> GetEventSet()->subscribeEvent(EventPropGridValueChanged::EventID,Subscriber(&SDesignerView::OnPropGridValueChanged,this));
				m_pPropgrid-> GetEventSet()->subscribeEvent(EventPropGridItemClick::EventID,Subscriber(&SDesignerView::OnPropGridItemClick,this));


			}

			m_strCurrentCtrlType = strCtrlType;

	//}
}


void SDesignerView::UpdatePropGrid(pugi::xml_node xmlNode)
{
	if (m_pPropgrid == NULL)
	{
		return;
	}

	m_pPropgrid->ClearAllGridItemValue();

	if (xmlNode == m_CurrentLayoutNode)
	{
		if (_wcsicmp(xmlNode.name(), _T("SOUI")) == 0)
		{
			pugi::xml_attribute xmlAttr = xmlNode.first_attribute();

			while (xmlAttr)
			{
				SStringT str = xmlAttr.name();
				IPropertyItem *pItem = m_pPropgrid->GetGridItem(str.MakeLower());
				if (pItem)
				{
					if (str.CompareNoCase(_T("name")) == 0)
					{
						SStringT sTemp = xmlAttr.value();
						SStringT sTemp1 = xmlNode.attribute(L"uidesiner_name").value();

						if (_wcsicmp(xmlAttr.value(), xmlNode.attribute(L"uidesiner_name").value()) != 0)
						{
							pItem->SetStringOnly(xmlNode.attribute(L"uidesiner_name").value());
						}
					}else
					{
						pItem->SetStringOnly(xmlAttr.value());
					}

				}

				xmlAttr = xmlAttr.next_attribute();
			}

			xmlAttr = xmlNode.child(_T("root")).first_attribute();
			while (xmlAttr)
			{
				SStringT str = xmlAttr.name();
				IPropertyItem *pItem = m_pPropgrid->GetGridItem(str.MakeLower());
				if (pItem)
				{
					if (str.CompareNoCase(_T("name")) == 0)
					{
						SStringT sTemp = xmlAttr.value();
						SStringT sTemp1 = xmlNode.attribute(L"uidesiner_name").value();

						if (_wcsicmp(xmlAttr.value(), xmlNode.attribute(L"uidesiner_name").value()) != 0)
						{
							pItem->SetStringOnly(xmlNode.attribute(L"uidesiner_name").value());
						}
					}else
					{
						pItem->SetStringOnly(xmlAttr.value());
					}

				}

				xmlAttr = xmlAttr.next_attribute();
			}

		}else
		{
			return;
		}

	}else
	{
		pugi::xml_attribute xmlAttr = xmlNode.first_attribute();

		IPropertyItem *pItem = m_pPropgrid->GetGridItem(_T("UiEdit_windowText"));
		if (pItem)
		{
			pItem->SetStringOnly(xmlNode.text().get());
		};

		while (xmlAttr)
		{
			SStringT str = xmlAttr.name();
			IPropertyItem *pItem = m_pPropgrid->GetGridItem(str.MakeLower());
			if (pItem)
			{
				if (str.CompareNoCase(_T("name")) == 0)
				{
					SStringT sTemp = xmlAttr.value();
					SStringT sTemp1 = xmlNode.attribute(L"uidesiner_name").value();

					if (_wcsicmp(xmlAttr.value(), xmlNode.attribute(L"uidesiner_name").value()) != 0)
					{
						pItem->SetStringOnly(xmlNode.attribute(L"uidesiner_name").value());
					}
				}else
				{
					pItem->SetStringOnly(xmlAttr.value());
				}



			}

			xmlAttr = xmlAttr.next_attribute();
		}
	}

	m_pPropgrid->Invalidate();
}


bool SDesignerView::OnPropGridValueChanged( EventArgs *pEvt )
{
	pugi:: xml_node xmlNode;
	BOOL bRoot = FALSE;

	IPropertyItem* pItem = ((EventPropGridValueChanged*)pEvt)->pItem;
	SStringT s = pItem->GetName2();  //��������pos skin name id �ȵ�

	SStringT s1 = pItem->GetString();   //���Ե�ֵ

	if (s.IsEmpty())
	{
		return false;
	}

	//�����ǰѡ����ǲ��ָ����ڣ���Ҫ���⴦��
	if (m_CurSelCtrl == m_pMoveWndRoot)
	{


		SPOSITION pos = m_lstRootProperty.GetHeadPosition();
		while (pos)
		{
			SStringT strTemp = m_lstRootProperty.GetNext(pos);
			if (strTemp.CompareNoCase(s) == 0)
			{
				xmlNode = m_xmlNode.child(_T("root"));
				bRoot = TRUE;
				break;
			}
			
		}

		if (!bRoot)
		{
			SPOSITION pos = m_lstSouiProperty.GetHeadPosition();
			while (pos)
			{
				SStringT strTemp = m_lstSouiProperty.GetNext(pos);
				if (strTemp.CompareNoCase(s) == 0)
				{
					xmlNode = m_xmlNode;
				    bRoot = TRUE;
					break;
				}

			}
		}



	}else
	{
		xmlNode = m_xmlNode;
	}






	if (s.CompareNoCase(_T("UiEdit_windowText")) == 0)
	{
		xmlNode.text().set(s1);
	}

	SWindow * pWnd = ((SMoveWnd*)m_CurSelCtrl)->m_pRealWnd->GetParent();

	pugi::xml_attribute attr = xmlNode.attribute(s);
	if (attr)
	{
		if (s1.IsEmpty())
		{
			if (s.CompareNoCase(_T("name")) == 0)
			{
				xmlNode.attribute(_T("uidesiner_name")).set_value(xmlNode.attribute(_T("name")).value());

			}else
			{
			  xmlNode.remove_attribute(s);
			}
		}else
		{
			if (s.CompareNoCase(_T("name")) == 0)
			{
				xmlNode.attribute(_T("uidesiner_name")).set_value(s1);

			}else
			{
				attr.set_value(s1);
			}
		}
	}
	else
	{
		if ((!s1.IsEmpty()) && (s.CompareNoCase(_T("UiEdit_windowText")) != 0))
		{
			xmlNode.append_attribute(s).set_value(s1);
		}
	}

	/*SwndLayout *pLayout = pWnd->GetLayout();
	pLayout->Clear();

	pWnd->SetAttribute(s, s1);

	if (m_xmlNode.attribute(L"pos"))
	{
		pLayout->InitPosFromString(m_xmlNode.attribute(L"pos").value());
	}

	if (m_xmlNode.attribute(L"pos2type"))
	{
		pLayout->InitOffsetFromPos2Type(m_xmlNode.attribute(L"pos2type").value());
	}

	if (m_xmlNode.attribute(L"offset"))
	{
		pLayout->InitOffsetFromString(m_xmlNode.attribute(L"offset").value());
	}

	if (m_xmlNode.attribute(L"size"))
	{
		pLayout->InitSizeFromString(m_xmlNode.attribute(L"size").value());
	}*/


	SStringT strName = ((SMoveWnd*)m_CurSelCtrl)->m_pRealWnd->GetName();

    ReLoadLayout();

	if (bRoot)
	{
		m_CurSelCtrl = m_pMoveWndRoot;
	}else
	{
		SWindow *pRealWnd =m_pRealWndRoot->FindChildByName2<SWindow>(strName);

		SMap<SWindow*,SMoveWnd*>::CPair *p = m_mapMoveRealWnd.Lookup(pRealWnd);
		if (p)
		{	

			m_CurSelCtrl = p->m_value;
		}else
		{
			Debug(_T("������"));
		}
	}


	return true;
}

void SDesignerView::RefreshRes()
{
	m_xmlDocUiRes.load_file(m_strUIResFile);


	CAutoRefPtr<IResProvider>   pResProvider1;
	TCHAR *s = m_strProPath.GetBuffer(m_strProPath.GetLength());

    IResProvider*	pResProvider = SApplication::getSingletonPtr()->GetMatchResProvider(_T("UIDEF"),_T("XML_INIT"));

	SApplication::getSingletonPtr()->RemoveResProvider(pResProvider);

	CreateResProvider(RES_FILE, (IObjRef**)&pResProvider1);
	if (!pResProvider1->Init((LPARAM)s, 0))
	{
		Debug(_T("ResProvider��ʼ��ʧ��"));
		return ;
	}
	SApplication::getSingletonPtr()->AddResProvider(pResProvider1);
}


bool SDesignerView::OnPropGridItemClick( EventArgs *pEvt )
{

	EventPropGridItemClick *pEvent = (EventPropGridItemClick*)pEvt;
	IPropertyItem* pItem = pEvent->pItem;
	SStringT strType = pEvent->strType;

	if (strType.CompareNoCase(_T("skin")) == 0)
	{
		SDlgSkinSelect DlgSkin(_T("layout:UIDESIGNER_XML_SKIN_SELECT"), pItem->GetString(), m_strUIResFile);
		if (DlgSkin.DoModal(m_pMainHost->m_hWnd) == IDOK)
		{
			RefreshRes();
			pItem->SetString(DlgSkin.m_strSkinName);
			//ReLoadLayout();
		}
		
	    

		//����Ƥ���Ի���
	}
	else if (strType.CompareNoCase(_T("font")) == 0)
	{
	
		//�������ͶԻ���
	
			//((CMainDlg*)m_pMainHost)->test();


	}	else if (strType.CompareNoCase(_T("class")) == 0)
	{
	}



	return true;
}

BOOL SDesignerView::ReLoadLayout()
{
	pugi::xml_node xmlnode;
		BOOL bIsInclude = FALSE;

	if (m_CurrentLayoutNode == NULL)
	{
		return TRUE;
	}


	if (S_CW2T(m_CurrentLayoutNode.name()) != _T("SOUI"))
	{
		xmlnode = m_CurrentLayoutNode;
		bIsInclude = TRUE;
	}
	else
	{
		////����˽��Ƥ��
		//if (m_privateStylePool->GetCount())
		//{
		//	m_privateStylePool->RemoveAll();
		//	GETSTYLEPOOLMGR->PopStylePool(m_privateStylePool);
		//}
		//BOOL ret=m_privateStylePool->Init(m_CurrentLayoutNode.child(L"style"));
		//if (ret)
		//{
		//	GETSTYLEPOOLMGR->PushStylePool(m_privateStylePool);
		//}
		//if (m_privateSkinPool->GetCount())
		//{
		//	m_privateSkinPool->RemoveAll();
		//	GETSKINPOOLMGR->PopSkinPool(m_privateSkinPool);
		//}
		//int skincount=m_privateSkinPool->LoadSkins(m_CurrentLayoutNode.child(L"skin"));//��xmlNode�Ӽ���˽��skin
		//if (skincount)
		//{
		//	GETSKINPOOLMGR->PushSkinPool(m_privateSkinPool);
		//}

		xmlnode=m_CurrentLayoutNode.child(L"root", false);
	}
	if(!xmlnode) return FALSE;


	SWindow *pChild = m_pContainer->GetWindow(GSW_FIRSTCHILD);
	while(pChild)
	{
		SWindow *pNext = pChild->GetWindow(GSW_NEXTSIBLING);
		pChild->DestroyWindow();
		pChild = pNext;
	}


	SStringW s1, s2;

	if (!bIsInclude)
	{
		pugi::xml_attribute attr = m_CurrentLayoutNode.first_attribute();
		while (attr)
		{
			s1.Format(L" %s=\"%s\" ", attr.name(), attr.value());
			s2 = s2 + s1;

			attr = attr.next_attribute();
		}

		attr = xmlnode.first_attribute();
		while (attr)
		{
			s1.Format(L" %s=\"%s\" ", attr.name(), attr.value());
			s2 = s2 + s1;

			attr = attr.next_attribute();
		}

		s2 = L"<window pos=\"20,20\" " +  s2 + L"></window>";

	}else
	{

			s2 = L"<window pos=\"20,20,-20,-20\" colorBkgnd=\"#d0d0d0\"></window>";
	}

	//wchar_t *s = L"<window pos=\"20,20,@500,@500\" colorBkgnd=\"#d0d0d0\"></window>";
	wchar_t *s3 = L"<movewnd pos=\"20,20,@500,@500\" ></movewnd>";

	////�������ִ��ڵĸ�����

	m_pRealWndRoot = m_pContainer->CreateChildren(s2);
	m_pMoveWndRoot = (SMoveWnd *)m_pContainer->CreateChildren(s3);
	m_pMoveWndRoot->m_pRealWnd = m_pRealWndRoot;

	m_mapMoveRealWnd.RemoveAll();
	m_mapMoveRealWnd[m_pMoveWndRoot->m_pRealWnd] = m_pMoveWndRoot;



	m_pMoveWndRoot->m_Desiner = this;	

	if (!m_pRealWndRoot->CreateChildren(xmlnode))
	{
		return FALSE;
	}

	CreateAllChildWnd(m_pRealWndRoot, m_pMoveWndRoot);

	m_treeXmlStruct->RemoveAllItems();
	InitXMLStruct(m_CurrentLayoutNode, STVI_ROOT);

	return TRUE;
}



BOOL SDesignerView::bIsContainerCtrl(SStringT strCtrlName) //�жϿؼ��Ƿ��������ؼ�
{
	SStringT s;
	SPOSITION pos = m_lstContainerCtrl.GetHeadPosition();
	while (pos)
	{
		s = m_lstContainerCtrl.GetNext(pos);
		if (s.CompareNoCase(strCtrlName) == 0)
		{
			return TRUE;
		}

	}
	return FALSE;
}


void SDesignerView::AddCodeToEditor(CScintillaWnd* pSciWnd)  //����xml���뵽����༭��
{
	m_strCurFileEditor = m_strCurFile;

	pSciWnd->SendMessage(SCI_CLEARALL, 0, 0);

	SStringA str;

	pugi::xml_document doc;
	SStringT strNodeName(_T("include"));

	if (m_xmlNode == m_CurrentLayoutNode && strNodeName.CompareNoCase(m_xmlNode.name()) == 0)
	{
		return;
	}else
	if (m_xmlNode == m_CurrentLayoutNode)
	{
		doc.append_copy(m_xmlNode.child(_T("root")));
	}else
	{
		doc.append_copy(m_xmlNode);
	}

	//Debug(doc.root().first_child());

	RemoveWndName(doc.root());


	pugi::xml_writer_buff writer;
	doc.first_child().print(writer,L"\t",pugi::format_default,pugi::encoding_utf16);
	SStringW *strDebug= new SStringW(writer.buffer(),writer.size());

	str=S_CW2A(*strDebug,CP_UTF8);
	pSciWnd->SendMessage(SCI_ADDTEXT, str.GetLength(),
		reinterpret_cast<LPARAM>((LPCSTR)str));
	delete strDebug;
}


void SDesignerView::GetCodeFromEditor(CScintillaWnd* pSciWnd)//�Ӵ���༭����ȡxml����
{

	if (m_strCurFileEditor.CompareNoCase(m_strCurFile) != 0)
	{
		return;
	}
	

	int n = pSciWnd->SendMessage(SCI_GETTEXT, 0,
		0);

	if (n == 0)
	{
		return;
	}

	char *chText = new char[n];


	pSciWnd->SendMessage(SCI_GETTEXT, n, (LPARAM)chText);

	SStringA s(chText);
	SStringW s1 = S_CA2W(s, CP_UTF8);


	pugi::xml_document doc;
	if(!doc.load_buffer(s1,wcslen(s1)*sizeof(wchar_t),pugi::parse_default,pugi::encoding_utf16))
	{
		Debug(_T("XML���﷨����"));
		return;
	}

	RenameChildeWnd(doc.root());


	BOOL bRoot = FALSE;

	SStringT strNodeName(_T("include"));
	if (m_xmlNode == m_CurrentLayoutNode && strNodeName.CompareNoCase(m_xmlNode.name()) == 0)
	{
		return;
	}else
		if (m_xmlNode == m_CurrentLayoutNode)
		{
			pugi::xml_node fristNode = m_xmlNode.child(_T("root"));
			pugi::xml_node xmlNode = m_xmlNode.insert_copy_after(doc.root().first_child(), fristNode);
			m_xmlNode.remove_child(fristNode);
			bRoot = TRUE;

		}else
		{
			pugi::xml_node xmlPNode;
			xmlPNode = m_xmlNode.parent();
			pugi::xml_node xmlNode = xmlPNode.insert_copy_after(doc.root().first_child(), m_xmlNode);

			xmlPNode.remove_child(m_xmlNode);
			m_xmlNode = xmlNode;
		}


	ReLoadLayout();

	//SStringT strName = ((SMoveWnd*)m_CurSelCtrl)->m_pRealWnd->GetName();

	//ReLoadLayout();

	//if (bRoot)
	//{
	//	m_CurSelCtrl = m_pMoveWndRoot;
	//}else
	//{
	//	SWindow *pRealWnd =m_pRealWndRoot->FindChildByName2<SWindow>(strName);

	//	SMap<SWindow*,SMoveWnd*>::CPair *p = m_mapMoveRealWnd.Lookup(pRealWnd);
	//	if (p)
	//	{	

	//		m_CurSelCtrl = p->m_value;
	//	}else
	//	{
	//		Debug(_T("������"));
	//	}
	//}

	


}

void SDesignerView::SetSelCtrlNode(pugi::xml_node xmlNode)
{
	m_nState = 1;
	m_xmlSelCtrlNode = xmlNode;
	return;
}


void SDesignerView::NewWnd(CPoint pt, SMoveWnd *pM)
{
	BOOL bIsInclude = FALSE;

	m_xmlNode = m_xmlSelCtrlNode;

	//�滻Include
	if(_wcsicmp(m_xmlNode.name(),L"include")==0 && m_xmlNode.attribute(L"src"))
	{
		bIsInclude = TRUE;
		SStringT strInclude = NodeToStr(m_xmlNode);
		m_xmlNode.set_name(L"window");
		m_xmlNode.remove_attribute(L"src");
		m_xmlNode.append_attribute(L"pos").set_value(L"10,10,-10,-10");
		m_xmlNode.append_attribute(L"colorBkgnd").set_value(L"RGB(191,141,255)");
		m_xmlNode.append_attribute(L"colorBkgnd").set_value(L"RGB(191,141,255)");
		m_xmlNode.text().set(strInclude);

		RenameWnd(m_xmlNode, TRUE);
		m_mapInclude[m_xmlNode.attribute(L"name").value()] = strInclude;

	}else
	{

		//�������ؼ�
		RenameWnd(m_xmlNode, TRUE);
		RenameChildeWnd(m_xmlNode, TRUE);

	}



	SWindow* pRealWnd;
	SMoveWnd* pMoveWnd;

	if(pM->m_pRealWnd == m_pRealWndRoot)
	{
		pRealWnd = pM->m_pRealWnd;
		pMoveWnd = pM;
	}else
	{
		pugi::xml_node xmlNodeRealWnd = FindNodeByAttr(m_CurrentLayoutNode, L"name", pM->m_pRealWnd->GetName());
		if (bIsContainerCtrl(xmlNodeRealWnd.name()))
		{
			pRealWnd = pM->m_pRealWnd;
			pMoveWnd = pM;
		}else
		{
			pRealWnd = pM->m_pRealWnd->GetParent();
			pMoveWnd = (SMoveWnd*)pM->GetParent();
		}
	}


	if (!bIsInclude)
	{
		CRect rect;
		pMoveWnd->GetWindowRect(rect);

		SStringT strPos;
		/*strPos.Format(_T("%d,%d"), pt.x - rect.left, pt.y - rect.top);*/

		//8 ����
		strPos.Format(_T("%d,%d"), (pt.x - rect.left)/8*8, (pt.y - rect.top)/8*8);

		if (!m_xmlNode.attribute(L"pos"))
		{
			m_xmlNode.append_attribute(L"pos");
		}

		m_xmlNode.attribute(L"pos").set_value(strPos);

		if (m_xmlNode.attribute(L"size"))
		{
			SStringT strSize;
			strSize = m_xmlNode.attribute(L"size", false).value();
		}
	}



	SStringT strMoveWnd;
	strMoveWnd = _T("<movewnd pos=\"0,0,@120,@30\"></movewnd>");


	pugi::xml_writer_buff writer;
	//m_Desiner->m_xmlNode.print(writer,L"\t",pugi::format_default,pugi::encoding_utf16);
	m_xmlNode.print(writer,L"\t",pugi::format_default,pugi::encoding_utf16);
	SStringW *strxmlWnd= new SStringW(writer.buffer(),writer.size());



	pRealWnd->CreateChildren(*strxmlWnd);
	SWindow *Wnd = pRealWnd->GetWindow(GSW_LASTCHILD);
	SMoveWnd *Wnd1 = (SMoveWnd *)CreateWnd(pMoveWnd, strMoveWnd);
	Wnd1->m_pRealWnd = Wnd;

	m_mapMoveRealWnd[Wnd] = Wnd1;
	CreateAllChildWnd(Wnd, Wnd1);


	//�ҵ�m_realWnd�ؼ���Ӧ��xml�ڵ�
	if(pRealWnd == m_pRealWndRoot)
	{
		SStringT s(_T("SOUI"));
		pugi::xml_node firstNode = m_CurrentLayoutNode;
		
		if (s.CompareNoCase(firstNode.name()) == 0)
		{
			firstNode = firstNode.child(_T("root"));
		}
		SetCurrentCtrl(firstNode.append_copy(m_xmlNode), Wnd1);
		//m_Desiner->m_xmlNode = firstNode.append_copy(m_Desiner->m_xmlNode);
	}
	else
	{
		//�ҵ�m_realWnd�ؼ���Ӧ��xml�ڵ�
		pugi::xml_node xmlNodeRealWnd = FindNodeByAttr(m_CurrentLayoutNode, L"name", pRealWnd->GetName());

		//���´����Ŀؼ�д�븸�ؼ���xml�ڵ�
		SetCurrentCtrl(xmlNodeRealWnd.append_copy(m_xmlNode), Wnd1);
		//m_Desiner->m_xmlNode = xmlNodeRealWnd.append_copy(m_Desiner->m_xmlNode);
	}

	m_nState = 0;


	delete strxmlWnd;
}

void SDesignerView::InitXMLStruct(pugi::xml_node xmlNode, HSTREEITEM item)
{
	if (!xmlNode)
	{
		return;
	}

	pugi::xml_node NodeSib = xmlNode;
	while (NodeSib)
	{
		SStringT strName;
		if (NodeSib.attribute(_T("name")))
		{
			strName = NodeSib.attribute(_T("name")).value();
		}else
		{
			strName = _T("");
		}
		SStringT *strName1 = new SStringT(strName);

		SStringT strNodeName = NodeSib.name();
		if (strNodeName.IsEmpty())
		{
			return;
		}

		HSTREEITEM itemChild = m_treeXmlStruct->InsertItem(strNodeName, item);  
		m_treeXmlStruct->SetItemData(itemChild, (LPARAM)strName1);  


		InitXMLStruct(NodeSib.first_child(), itemChild);
		NodeSib = NodeSib.next_sibling();

	}
	m_treeXmlStruct->Invalidate();
}



BOOL SDesignerView::GoToXmlStructItem(SStringT strCtrlName, HSTREEITEM item)
{
	    HSTREEITEM SibItem = item;

		while (SibItem)
		{
			SStringT *s  = (SStringT*)m_treeXmlStruct->GetItemData(SibItem);

			if (s->CompareNoCase(strCtrlName) == 0)
			{
				m_treeXmlStruct->SelectItem(SibItem);
				m_treeXmlStruct->Invalidate();
				return TRUE;
			}

			HSTREEITEM ChildItem = m_treeXmlStruct->GetChildItem(SibItem);



		    BOOL Result = GoToXmlStructItem(strCtrlName, ChildItem);
			if (Result)
			{
				return TRUE;
			}

			SibItem = m_treeXmlStruct->GetNextSiblingItem(SibItem);

		}
		return FALSE;
}


bool SDesignerView::OnTCSelChanged(EventArgs *pEvt)
{
	if (!m_pContainer->GetParent()->IsVisible())
	{   //������д�ɣ���ʱ���ٸ�
		return true;
	}

	EventTCSelChanged *evt = (EventTCSelChanged*)pEvt;

	HSTREEITEM item =  m_treeXmlStruct->GetSelectedItem();

	SStringT *s = (SStringT*)m_treeXmlStruct->GetItemData(item);

	pugi::xml_node xmlNode = FindNodeByAttr(m_CurrentLayoutNode, L"name", *s);

    if (!xmlNode)
    {
		return true;
    }

	
	SWindow *wnd = m_pRealWndRoot->FindChildByName(*s);

	if (wnd)
	{
		SMap<SWindow*, SMoveWnd*>::CPair *p = m_mapMoveRealWnd.Lookup(wnd);
		if (p)
		{
			m_xmlNode = xmlNode;
			m_CurSelCtrl = p->m_value;

			m_CurSelCtrl->Click(0,CPoint(0,0));
		}


	}



	return true;
}


void SDesignerView::DeleteCtrl()
{
	if (m_xmlNode == m_CurrentLayoutNode)
	{
		return;
	}else
	{
		m_xmlNode.parent().remove_child(m_xmlNode);

		//Debug(m_CurrentLayoutNode);
		ReLoadLayout();
		m_nState = 0;
		GetMoveWndRoot()->Click(0, CPoint(0,0));
	}
}

void SDesignerView::Preview()
{
	//SMap<SWindow*, SMoveWnd*>::CPair *p = m_mapMoveRealWnd.GetNext();

	SMoveWnd *wnd;

	m_pMoveWndRoot->SetVisible(FALSE);
	//SPOSITION pos = m_mapMoveRealWnd.GetStartPosition();
	//while (pos)
	//{
	//	SMap<SWindow*, SMoveWnd*>::CPair *p = m_mapMoveRealWnd.GetNext(pos);
	//	wnd = p->m_value;
	//	wnd->SetVisible(FALSE);
	//}
	
}

void SDesignerView::unPreview()
{
	ReLoadLayout();
	m_nState = 0;
	GetMoveWndRoot()->Click(0, CPoint(0,0));
}

void SDesignerView::ShowZYGLDlg()
{
	if (m_strUIResFile.IsEmpty())
	{
		return;
	}
	SDlgSkinSelect DlgSkin(_T("layout:UIDESIGNER_XML_SKIN_SELECT"), _T(""), m_strUIResFile, FALSE);
	if (DlgSkin.DoModal(m_pMainHost->m_hWnd) == IDOK)
	{
		RefreshRes();
	}
}

void SDesignerView::ShowYSGLDlg()
{
	if (m_strUIResFile.IsEmpty())
	{
		return;
	}
	SDlgStyleManage dlg;
	if (dlg.DoModal(m_pMainHost->m_hWnd) == IDOK)
	{
		RefreshRes();
	}
}