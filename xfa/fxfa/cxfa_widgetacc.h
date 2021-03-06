// Copyright 2014 PDFium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#ifndef XFA_FXFA_CXFA_WIDGETACC_H_
#define XFA_FXFA_CXFA_WIDGETACC_H_

#include <memory>

#include "core/fxcrt/cfx_retain_ptr.h"
#include "core/fxcrt/fx_coordinates.h"
#include "core/fxge/dib/cfx_dibitmap.h"
#include "core/fxge/fx_dib.h"
#include "xfa/fxfa/parser/cxfa_box.h"
#include "xfa/fxfa/parser/cxfa_event.h"
#include "xfa/fxfa/parser/cxfa_image.h"
#include "xfa/fxfa/parser/cxfa_margin.h"
#include "xfa/fxfa/parser/cxfa_script.h"
#include "xfa/fxfa/parser/cxfa_value.h"
#include "xfa/fxfa/parser/cxfa_widgetdata.h"

class CFGAS_GEFont;
class CXFA_EventParam;
class CXFA_FFApp;
class CXFA_FFDoc;
class CXFA_FFDocView;
class CXFA_FFWidget;
class CXFA_Node;
class CXFA_TextLayout;
class CXFA_WidgetLayoutData;
class IXFA_AppProvider;

class CXFA_WidgetAcc : public CXFA_WidgetData {
 public:
  CXFA_WidgetAcc(CXFA_FFDocView* pDocView, CXFA_Node* pNode);
  ~CXFA_WidgetAcc();

  bool GetName(CFX_WideString& wsName, int32_t iNameType = 0);
  bool ProcessValueChanged();
  void ResetData();

  void SetImageEdit(const CFX_WideString& wsContentType,
                    const CFX_WideString& wsHref,
                    const CFX_WideString& wsData);

  CXFA_WidgetAcc* GetExclGroup();
  CXFA_FFDocView* GetDocView();
  CXFA_FFDoc* GetDoc();
  CXFA_FFApp* GetApp();
  IXFA_AppProvider* GetAppProvider();

  int32_t ProcessEvent(int32_t iActivity, CXFA_EventParam* pEventParam);
  int32_t ProcessEvent(const CXFA_Event& event, CXFA_EventParam* pEventParam);
  int32_t ProcessCalculate();
  int32_t ProcessValidate(int32_t iFlags = 0);
  int32_t ExecuteScript(CXFA_Script script,
                        CXFA_EventParam* pEventParam,
                        CFXJSE_Value** pRetValue = nullptr);

  CXFA_FFWidget* GetNextWidget(CXFA_FFWidget* pWidget);
  void StartWidgetLayout(float& fCalcWidth, float& fCalcHeight);
  bool FindSplitPos(int32_t iBlockIndex, float& fCalcHeight);
  bool LoadCaption();
  void LoadText();
  bool LoadImageImage();
  bool LoadImageEditImage();
  void GetImageDpi(int32_t& iImageXDpi, int32_t& iImageYDpi);
  void GetImageEditDpi(int32_t& iImageXDpi, int32_t& iImageYDpi);
  CXFA_TextLayout* GetCaptionTextLayout();
  CXFA_TextLayout* GetTextLayout();
  CFX_RetainPtr<CFX_DIBitmap> GetImageImage();
  CFX_RetainPtr<CFX_DIBitmap> GetImageEditImage();
  void SetImageImage(const CFX_RetainPtr<CFX_DIBitmap>& newImage);
  void SetImageEditImage(const CFX_RetainPtr<CFX_DIBitmap>& newImage);
  void UpdateUIDisplay(CXFA_FFWidget* pExcept = nullptr);

  CXFA_Node* GetDatasets();
  CFX_RetainPtr<CFGAS_GEFont> GetFDEFont();
  float GetFontSize();
  FX_ARGB GetTextColor();
  float GetLineHeight();
  CXFA_WidgetLayoutData* GetWidgetLayoutData();

 private:
  void ProcessScriptTestValidate(CXFA_Validate validate,
                                 int32_t iRet,
                                 CFXJSE_Value* pRetValue,
                                 bool bVersionFlag);
  int32_t ProcessFormatTestValidate(CXFA_Validate validate, bool bVersionFlag);
  int32_t ProcessNullTestValidate(CXFA_Validate validate,
                                  int32_t iFlags,
                                  bool bVersionFlag);
  CFX_WideString GetValidateCaptionName(bool bVersionFlag);
  CFX_WideString GetValidateMessage(bool bError, bool bVersionFlag);
  void CalcCaptionSize(CFX_SizeF& szCap);
  bool CalculateFieldAutoSize(CFX_SizeF& size);
  bool CalculateWidgetAutoSize(CFX_SizeF& size);
  bool CalculateTextEditAutoSize(CFX_SizeF& size);
  bool CalculateCheckButtonAutoSize(CFX_SizeF& size);
  bool CalculatePushButtonAutoSize(CFX_SizeF& size);
  bool CalculateImageEditAutoSize(CFX_SizeF& size);
  bool CalculateImageAutoSize(CFX_SizeF& size);
  bool CalculateTextAutoSize(CFX_SizeF& size);
  float CalculateWidgetAutoHeight(float fHeightCalc);
  float CalculateWidgetAutoWidth(float fWidthCalc);
  float GetWidthWithoutMargin(float fWidthCalc);
  float GetHeightWithoutMargin(float fHeightCalc);
  void CalculateTextContentSize(CFX_SizeF& size);
  void CalculateAccWidthAndHeight(XFA_Element eUIType,
                                  float& fWidth,
                                  float& fCalcHeight);
  void InitLayoutData();
  void StartTextLayout(float& fCalcWidth, float& fCalcHeight);

  CXFA_FFDocView* m_pDocView;
  std::unique_ptr<CXFA_WidgetLayoutData> m_pLayoutData;
  uint32_t m_nRecursionDepth;
};

#endif  // XFA_FXFA_CXFA_WIDGETACC_H_
