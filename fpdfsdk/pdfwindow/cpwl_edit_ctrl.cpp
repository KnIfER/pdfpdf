// Copyright 2014 PDFium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#include "fpdfsdk/pdfwindow/cpwl_edit_ctrl.h"

#include "core/fpdfdoc/cpvt_section.h"
#include "core/fpdfdoc/cpvt_word.h"
#include "core/fxge/fx_font.h"
#include "fpdfsdk/fxedit/fxet_edit.h"
#include "fpdfsdk/pdfwindow/cpwl_caret.h"
#include "fpdfsdk/pdfwindow/cpwl_font_map.h"
#include "fpdfsdk/pdfwindow/cpwl_scroll_bar.h"
#include "fpdfsdk/pdfwindow/cpwl_utils.h"
#include "fpdfsdk/pdfwindow/cpwl_wnd.h"
#include "public/fpdf_fwlevent.h"

CPWL_EditCtrl::CPWL_EditCtrl()
    : m_pEdit(new CFX_Edit),
      m_pEditCaret(nullptr),
      m_bMouseDown(false),
      m_nCharSet(FX_CHARSET_Default) {}

CPWL_EditCtrl::~CPWL_EditCtrl() {}

void CPWL_EditCtrl::OnCreate(PWL_CREATEPARAM& cp) {
  cp.eCursorType = FXCT_VBEAM;
}

void CPWL_EditCtrl::OnCreated() {
  SetFontSize(GetCreationParam().fFontSize);

  m_pEdit->SetFontMap(GetFontMap());
  m_pEdit->SetNotify(this);
  m_pEdit->Initialize();
}

bool CPWL_EditCtrl::IsWndHorV() {
  CFX_Matrix mt = GetWindowMatrix();
  return mt.Transform(CFX_PointF(1, 1)).y == mt.Transform(CFX_PointF(0, 1)).y;
}

void CPWL_EditCtrl::SetCursor() {
  if (IsValid()) {
    if (CFX_SystemHandler* pSH = GetSystemHandler()) {
      if (IsWndHorV())
        pSH->SetCursor(FXCT_VBEAM);
      else
        pSH->SetCursor(FXCT_HBEAM);
    }
  }
}

CFX_WideString CPWL_EditCtrl::GetSelectedText() {
  if (m_pEdit)
    return m_pEdit->GetSelText();

  return CFX_WideString();
}

void CPWL_EditCtrl::RePosChildWnd() {
  m_pEdit->SetPlateRect(GetClientRect());
}

void CPWL_EditCtrl::SetScrollInfo(const PWL_SCROLL_INFO& info) {
  if (CPWL_Wnd* pChild = GetVScrollBar())
    pChild->SetScrollInfo(info);
}

void CPWL_EditCtrl::SetScrollPosition(float pos) {
  if (CPWL_Wnd* pChild = GetVScrollBar())
    pChild->SetScrollPosition(pos);
}

void CPWL_EditCtrl::ScrollWindowVertically(float pos) {
  m_pEdit->SetScrollPos(CFX_PointF(m_pEdit->GetScrollPos().x, pos));
}

void CPWL_EditCtrl::CreateChildWnd(const PWL_CREATEPARAM& cp) {
  if (!IsReadOnly())
    CreateEditCaret(cp);
}

void CPWL_EditCtrl::CreateEditCaret(const PWL_CREATEPARAM& cp) {
  if (m_pEditCaret)
    return;

  m_pEditCaret = new CPWL_Caret;
  m_pEditCaret->SetInvalidRect(GetClientRect());

  PWL_CREATEPARAM ecp = cp;
  ecp.pParentWnd = this;
  ecp.dwFlags = PWS_CHILD | PWS_NOREFRESHCLIP;
  ecp.dwBorderWidth = 0;
  ecp.nBorderStyle = BorderStyle::SOLID;
  ecp.rcRectWnd = CFX_FloatRect();

  m_pEditCaret->Create(ecp);
}

void CPWL_EditCtrl::SetFontSize(float fFontSize) {
  m_pEdit->SetFontSize(fFontSize);
}

float CPWL_EditCtrl::GetFontSize() const {
  return m_pEdit->GetFontSize();
}

bool CPWL_EditCtrl::OnKeyDown(uint16_t nChar, uint32_t nFlag) {
  if (m_bMouseDown)
    return true;

  bool bRet = CPWL_Wnd::OnKeyDown(nChar, nFlag);

  // FILTER
  switch (nChar) {
    default:
      return false;
    case FWL_VKEY_Delete:
    case FWL_VKEY_Up:
    case FWL_VKEY_Down:
    case FWL_VKEY_Left:
    case FWL_VKEY_Right:
    case FWL_VKEY_Home:
    case FWL_VKEY_End:
    case FWL_VKEY_Insert:
    case 'C':
    case 'V':
    case 'X':
    case 'A':
    case 'Z':
    case 'c':
    case 'v':
    case 'x':
    case 'a':
    case 'z':
      break;
  }

  if (nChar == FWL_VKEY_Delete && m_pEdit->IsSelected())
    nChar = FWL_VKEY_Unknown;

  switch (nChar) {
    case FWL_VKEY_Delete:
      Delete();
      return true;
    case FWL_VKEY_Insert:
      if (IsSHIFTpressed(nFlag))
        PasteText();
      return true;
    case FWL_VKEY_Up:
      m_pEdit->OnVK_UP(IsSHIFTpressed(nFlag), false);
      return true;
    case FWL_VKEY_Down:
      m_pEdit->OnVK_DOWN(IsSHIFTpressed(nFlag), false);
      return true;
    case FWL_VKEY_Left:
      m_pEdit->OnVK_LEFT(IsSHIFTpressed(nFlag), false);
      return true;
    case FWL_VKEY_Right:
      m_pEdit->OnVK_RIGHT(IsSHIFTpressed(nFlag), false);
      return true;
    case FWL_VKEY_Home:
      m_pEdit->OnVK_HOME(IsSHIFTpressed(nFlag), IsCTRLpressed(nFlag));
      return true;
    case FWL_VKEY_End:
      m_pEdit->OnVK_END(IsSHIFTpressed(nFlag), IsCTRLpressed(nFlag));
      return true;
    case FWL_VKEY_Unknown:
      if (!IsSHIFTpressed(nFlag))
        Clear();
      else
        CutText();
      return true;
    default:
      break;
  }

  return bRet;
}

bool CPWL_EditCtrl::OnChar(uint16_t nChar, uint32_t nFlag) {
  if (m_bMouseDown)
    return true;

  CPWL_Wnd::OnChar(nChar, nFlag);

  // FILTER
  switch (nChar) {
    case 0x0A:
    case 0x1B:
      return false;
    default:
      break;
  }

  bool bCtrl = IsCTRLpressed(nFlag);
  bool bAlt = IsALTpressed(nFlag);
  bool bShift = IsSHIFTpressed(nFlag);

  uint16_t word = nChar;

  if (bCtrl && !bAlt) {
    switch (nChar) {
      case 'C' - 'A' + 1:
        CopyText();
        return true;
      case 'V' - 'A' + 1:
        PasteText();
        return true;
      case 'X' - 'A' + 1:
        CutText();
        return true;
      case 'A' - 'A' + 1:
        SelectAll();
        return true;
      case 'Z' - 'A' + 1:
        if (bShift)
          Redo();
        else
          Undo();
        return true;
      default:
        if (nChar < 32)
          return false;
    }
  }

  if (IsReadOnly())
    return true;

  if (m_pEdit->IsSelected() && word == FWL_VKEY_Back)
    word = FWL_VKEY_Unknown;

  Clear();

  switch (word) {
    case FWL_VKEY_Back:
      Backspace();
      break;
    case FWL_VKEY_Return:
      InsertReturn();
      break;
    case FWL_VKEY_Unknown:
      break;
    default:
      InsertWord(word, GetCharSet());
      break;
  }

  return true;
}

bool CPWL_EditCtrl::OnLButtonDown(const CFX_PointF& point, uint32_t nFlag) {
  CPWL_Wnd::OnLButtonDown(point, nFlag);

  if (ClientHitTest(point)) {
    if (m_bMouseDown)
      InvalidateRect();

    m_bMouseDown = true;
    SetCapture();

    m_pEdit->OnMouseDown(point, IsSHIFTpressed(nFlag), IsCTRLpressed(nFlag));
  }

  return true;
}

bool CPWL_EditCtrl::OnLButtonUp(const CFX_PointF& point, uint32_t nFlag) {
  CPWL_Wnd::OnLButtonUp(point, nFlag);

  if (m_bMouseDown) {
    // can receive keybord message
    if (ClientHitTest(point) && !IsFocused())
      SetFocus();

    ReleaseCapture();
    m_bMouseDown = false;
  }

  return true;
}

bool CPWL_EditCtrl::OnMouseMove(const CFX_PointF& point, uint32_t nFlag) {
  CPWL_Wnd::OnMouseMove(point, nFlag);

  if (m_bMouseDown)
    m_pEdit->OnMouseMove(point, false, false);

  return true;
}

void CPWL_EditCtrl::SetEditCaret(bool bVisible) {
  CFX_PointF ptHead;
  CFX_PointF ptFoot;
  if (bVisible)
    GetCaretInfo(&ptHead, &ptFoot);

  SetCaret(bVisible, ptHead, ptFoot);
}

void CPWL_EditCtrl::GetCaretInfo(CFX_PointF* ptHead, CFX_PointF* ptFoot) const {
  CFX_Edit_Iterator* pIterator = m_pEdit->GetIterator();
  pIterator->SetAt(m_pEdit->GetCaret());
  CPVT_Word word;
  CPVT_Line line;
  if (pIterator->GetWord(word)) {
    ptHead->x = word.ptWord.x + word.fWidth;
    ptHead->y = word.ptWord.y + word.fAscent;
    ptFoot->x = word.ptWord.x + word.fWidth;
    ptFoot->y = word.ptWord.y + word.fDescent;
  } else if (pIterator->GetLine(line)) {
    ptHead->x = line.ptLine.x;
    ptHead->y = line.ptLine.y + line.fLineAscent;
    ptFoot->x = line.ptLine.x;
    ptFoot->y = line.ptLine.y + line.fLineDescent;
  }
}

void CPWL_EditCtrl::SetCaret(bool bVisible,
                             const CFX_PointF& ptHead,
                             const CFX_PointF& ptFoot) {
  if (m_pEditCaret) {
    if (!IsFocused() || m_pEdit->IsSelected())
      bVisible = false;

    m_pEditCaret->SetCaret(bVisible, ptHead, ptFoot);
  }
}

CFX_WideString CPWL_EditCtrl::GetText() const {
  return m_pEdit->GetText();
}

void CPWL_EditCtrl::SetSel(int32_t nStartChar, int32_t nEndChar) {
  m_pEdit->SetSel(nStartChar, nEndChar);
}

void CPWL_EditCtrl::GetSel(int32_t& nStartChar, int32_t& nEndChar) const {
  m_pEdit->GetSel(nStartChar, nEndChar);
}

void CPWL_EditCtrl::Clear() {
  if (!IsReadOnly())
    m_pEdit->Clear();
}

void CPWL_EditCtrl::SelectAll() {
  m_pEdit->SelectAll();
}

void CPWL_EditCtrl::SetScrollPos(const CFX_PointF& point) {
  m_pEdit->SetScrollPos(point);
}

CFX_PointF CPWL_EditCtrl::GetScrollPos() const {
  return m_pEdit->GetScrollPos();
}

void CPWL_EditCtrl::CopyText() {}

void CPWL_EditCtrl::PasteText() {}

void CPWL_EditCtrl::CutText() {}

void CPWL_EditCtrl::InsertWord(uint16_t word, int32_t nCharset) {
  if (!IsReadOnly())
    m_pEdit->InsertWord(word, nCharset);
}

void CPWL_EditCtrl::InsertReturn() {
  if (!IsReadOnly())
    m_pEdit->InsertReturn();
}

void CPWL_EditCtrl::Delete() {
  if (!IsReadOnly())
    m_pEdit->Delete();
}

void CPWL_EditCtrl::Backspace() {
  if (!IsReadOnly())
    m_pEdit->Backspace();
}

bool CPWL_EditCtrl::CanUndo() const {
  return !IsReadOnly() && m_pEdit->CanUndo();
}

bool CPWL_EditCtrl::CanRedo() const {
  return !IsReadOnly() && m_pEdit->CanRedo();
}

void CPWL_EditCtrl::Redo() {
  if (CanRedo())
    m_pEdit->Redo();
}

void CPWL_EditCtrl::Undo() {
  if (CanUndo())
    m_pEdit->Undo();
}

int32_t CPWL_EditCtrl::GetCharSet() const {
  return m_nCharSet < 0 ? FX_CHARSET_Default : m_nCharSet;
}

void CPWL_EditCtrl::SetReadyToInput() {
  if (m_bMouseDown) {
    ReleaseCapture();
    m_bMouseDown = false;
  }
}
