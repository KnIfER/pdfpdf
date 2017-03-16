// Copyright 2017 PDFium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#include "xfa/fgas/layout/cfx_break.h"

#include <algorithm>

namespace {

const int kMinimumTabWidth = 160000;

}  // namespace

CFX_Break::CFX_Break(uint32_t dwLayoutStyles)
    : m_eCharType(FX_CHARTYPE_Unknown),
      m_bSingleLine(false),
      m_bCombText(false),
      m_dwLayoutStyles(dwLayoutStyles),
      m_iLineStart(0),
      m_iLineWidth(2000000),
      m_wParagraphBreakChar(L'\n'),
      m_iFontSize(240),
      m_iTabWidth(720000),
      m_iHorizontalScale(100),
      m_iVerticalScale(100),
      m_iTolerance(0),
      m_iCharSpace(0),
      m_iDefChar(0),
      m_wDefChar(0xFEFF),
      m_pFont(nullptr),
      m_pCurLine(nullptr),
      m_iReadyLineIndex(-1) {
  m_pCurLine = &m_Line[0];
}

CFX_Break::~CFX_Break() {}

void CFX_Break::SetLayoutStyles(uint32_t dwLayoutStyles) {
  m_dwLayoutStyles = dwLayoutStyles;
  m_bSingleLine = (m_dwLayoutStyles & FX_LAYOUTSTYLE_SingleLine) != 0;
  m_bCombText = (m_dwLayoutStyles & FX_LAYOUTSTYLE_CombText) != 0;

  ResetArabicContext();
}

void CFX_Break::SetHorizontalScale(int32_t iScale) {
  iScale = std::max(iScale, 0);
  if (m_iHorizontalScale == iScale)
    return;

  SetBreakStatus();
  m_iHorizontalScale = iScale;
}

void CFX_Break::SetVerticalScale(int32_t iScale) {
  if (iScale < 0)
    iScale = 0;
  if (m_iVerticalScale == iScale)
    return;

  SetBreakStatus();
  m_iVerticalScale = iScale;
}

void CFX_Break::SetFont(const CFX_RetainPtr<CFGAS_GEFont>& pFont) {
  if (!pFont || pFont == m_pFont)
    return;

  SetBreakStatus();
  m_pFont = pFont;
  FontChanged();
}

void CFX_Break::SetFontSize(float fFontSize) {
  int32_t iFontSize = FXSYS_round(fFontSize * 20.0f);
  if (m_iFontSize == iFontSize)
    return;

  SetBreakStatus();
  m_iFontSize = iFontSize;
  FontChanged();
}

void CFX_Break::FontChanged() {
  m_iDefChar = 0;
  if (!m_pFont || m_wDefChar == 0xFEFF)
    return;

  m_pFont->GetCharWidth(m_wDefChar, m_iDefChar, false);
  m_iDefChar *= m_iFontSize;
}

void CFX_Break::SetTabWidth(float fTabWidth) {
  // Note, the use of max here was only done in the TxtBreak code. Leaving this
  // in for the RTFBreak code for consistency. If we see issues with tab widths
  // we may need to fix this.
  m_iTabWidth = std::max(FXSYS_round(fTabWidth * 20000.0f), kMinimumTabWidth);
}

void CFX_Break::SetDefaultChar(wchar_t wch) {
  m_wDefChar = wch;
  m_iDefChar = 0;
  if (m_wDefChar == 0xFEFF || !m_pFont)
    return;

  m_pFont->GetCharWidth(m_wDefChar, m_iDefChar, false);
  if (m_iDefChar < 0)
    m_iDefChar = 0;
  else
    m_iDefChar *= m_iFontSize;
}

void CFX_Break::SetParagraphBreakChar(wchar_t wch) {
  if (wch != L'\r' && wch != L'\n')
    return;
  m_wParagraphBreakChar = wch;
}

void CFX_Break::SetLineBreakTolerance(float fTolerance) {
  m_iTolerance = FXSYS_round(fTolerance * 20000.0f);
}

void CFX_Break::SetCharSpace(float fCharSpace) {
  m_iCharSpace = FXSYS_round(fCharSpace * 20000.0f);
}

void CFX_Break::SetLineBoundary(float fLineStart, float fLineEnd) {
  if (fLineStart > fLineEnd)
    return;

  m_iLineStart = FXSYS_round(fLineStart * 20000.0f);
  m_iLineWidth = FXSYS_round(fLineEnd * 20000.0f);
  m_pCurLine->m_iStart = std::min(m_pCurLine->m_iStart, m_iLineWidth);
  m_pCurLine->m_iStart = std::max(m_pCurLine->m_iStart, m_iLineStart);
}