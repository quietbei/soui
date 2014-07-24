######################################################################
# Automatically generated by qmake (2.01a) ?? ?? 23 19:29:25 2014
######################################################################

TEMPLATE = lib
TARGET = soui

DLL_SOUI{
    DEFINES += DLL_SOUI SOUI_EXPORTS
}
else{
    CONFIG += staticlib
}

dir = ..
include($$dir/common.pri)

CONFIG(debug,debug|release){
	DESTDIR = $$dir/bin/debug
	QMAKE_LIBDIR += $$DESTDIR
	LIBS += utilitiesd.lib
}
else{
	DESTDIR = $$dir/bin/release
	QMAKE_LIBDIR += $$DESTDIR
	LIBS += utilities.lib
}

PRECOMPILED_HEADER = souistd.h

DEPENDPATH += . \
              include \

INCLUDEPATH += . \
              include \
              ../utilities/include \


# Input
HEADERS += include/SApp.h \
           include/activex/SAxHost.h \
           include/activex/SBStr.h \
           include/control/SActiveX.h \
           include/control/SCalendar.h \
           include/control/SCaption.h \
           include/control/SCmnCtrl.h \
           include/control/SComboBox.h \
           include/control/SDateTimeEdit.h \
           include/control/SDropDown.h \
           include/control/SHeaderCtrl.h \
           include/control/SHotKeyCtrl.h \
           include/control/SItembox.h \
           include/control/SListbox.h \
           include/control/SListboxex.h \
           include/control/SListCtrl.h \
           include/control/SMessageBox.h \
           include/control/SouiCtrls.h \
           include/control/SRealWnd.h \
           include/control/SRichEdit.h \
           include/control/SScrollbar.h \
           include/control/SSliderBar.h \
           include/control/SSplitWnd.h \
           include/control/STabCtrl.h \
           include/control/stree.hpp \
           include/control/STreeBox.h \
           include/control/STreeCtrl.h \
           include/core/Accelerator.h \
           include/core/FocusManager.h \
           include/core/mybuffer.h \
           include/core/SDefine.h \
           include/core/SDropTargetDispatcher.h \
           include/core/SHostDialog.h \
           include/core/SHostWnd.h \
           include/core/SimpleWnd.h \
           include/core/SItemPanel.h \
           include/core/SMsgLoop.h \
           include/core/Sobject.h \
           include/core/SPanel.h \
           include/core/SSingleton.h \
           include/core/SSingletonMap.h \
           include/core/SSkin.h \
           include/core/SSkinFactoryMgr.h \
           include/core/SThreadActiveWndMgr.h \
           include/core/SWindowMgr.h \
           include/core/Swnd.h \
           include/core/SwndContainer-i.h \
           include/core/SwndContainerImpl.h \
           include/core/SwndFactoryMgr.h \
           include/core/SwndLayout.h \
           include/core/SwndStyle.h \
           include/event/eventcrack.h \
           include/event/Events.h \
           include/event/EventSet.h \
           include/event/EventSubscriber.h \
           include/helper/auto_reset.h \
           include/helper/color.h \
           include/helper/DragWnd.h \
           include/helper/MemDC.h \
           include/helper/MenuWndHook.h \
           include/helper/SAttrCracker.h \
           include/helper/SMenu.h \
           include/helper/SplitString.h \
           include/helper/SResID.h \
           include/helper/STime.h \
           include/helper/STimerEx.h \
           include/helper/STipCtrl.h \
           include/helper/SwndMsgCracker.h \
           include/interface/imgdecoder-i.h \
           include/interface/render-i.h \
           include/interface/SResProvider-i.h \
           include/interface/SScriptModule-i.h \
           include/interface/SSkinobj-i.h \
           include/interface/STranslator-i.h \
           include/res.mgr/SFontPool.h \
           include/res.mgr/SImgPool.h \
           include/res.mgr/SObjDefAttr.h \
           include/res.mgr/SResProvider.h \
           include/res.mgr/SResProviderMgr.h \
           include/res.mgr/SSkinPool.h \
           include/res.mgr/SStringPool.h \
           include/res.mgr/SStylePool.h \
           src/activex/SAxContainer.h \
           src/activex/SAxUtil.h \
           include/activex/flash10t.tlh \
           include/activex/flash10t.tli \
           include/activex/wmp.tlh

SOURCES += src/SApp.cpp \
           src/activex/SAxContainer.cpp \
           src/activex/SBStr.cpp \
           src/controls/SActiveX.cpp \
           src/controls/SCalendar.cpp \
           src/controls/SCaption.cpp \
           src/controls/SCmnCtrl.cpp \
           src/controls/SComboBox.cpp \
           src/controls/SDateTimeEdit.cpp \
           src/controls/SDropDown.cpp \
           src/controls/SHeaderCtrl.cpp \
           src/controls/SHotKeyCtrl.cpp \
           #src/controls/SItembox.cpp \
           src/controls/SListbox.cpp \
           src/controls/SListboxex.cpp \
           src/controls/SListCtrl.cpp \
           src/controls/SMessageBox.cpp \
           src/controls/SRealWnd.cpp \
           src/controls/SRichEdit.cpp \
           src/controls/SScrollbar.cpp \
           src/controls/SSliderBar.cpp \
           src/controls/SSplitWnd.cpp \
           src/controls/STabCtrl.cpp \
           src/controls/STreeBox.cpp \
           src/controls/STreeCtrl.cpp \
           src/core/Accelerator.cpp \
           src/core/FocusManager.cpp \
           src/core/SDropTargetDispatcher.cpp \
           src/core/SHostDialog.cpp \
           src/core/shostwnd.cpp \
           src/core/SimpleWnd.cpp \
           src/core/SItemPanel.cpp \
           src/core/SMsgLoop.cpp \
           src/core/SObject.cpp \
           src/core/SPanel.cpp \
           src/core/SSkin.cpp \
           src/core/SSkinFactoryMgr.cpp \
           src/core/SThreadActiveWndMgr.cpp \
           src/core/STipCtrl.cpp \
           src/core/SWindowMgr.cpp \
           src/core/Swnd.cpp \
           src/core/SwndContainerImpl.cpp \
           src/core/SWndFactoryMgr.cpp \
           src/core/SwndLayout.cpp \
           src/core/SwndStyle.cpp \
           src/event/Events.cpp \
           src/event/EventSet.cpp \
           src/helper/DragWnd.cpp \
           src/helper/MemDC.cpp \
           src/helper/MenuWndHook.cpp \
           src/helper/SMenu.cpp \
           src/helper/STimerEx.cpp \
           src/res.mgr/SFontPool.cpp \
           src/res.mgr/SImgPool.cpp \
           src/res.mgr/SObjDefAttr.cpp \
           src/res.mgr/SResProvider.cpp \
           src/res.mgr/SResProviderMgr.cpp \
           src/res.mgr/SSkinPool.cpp \
           src/res.mgr/SStringPool.cpp \
           src/res.mgr/SStylePool.cpp
