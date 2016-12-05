// Copyright 2014 PDFium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#include "xfa/fwl/core/cfwl_datetimeedit.h"

#include <memory>
#include <utility>

#include "third_party/base/ptr_util.h"
#include "xfa/fwl/core/cfwl_datetimepicker.h"
#include "xfa/fwl/core/cfwl_msgmouse.h"
#include "xfa/fwl/core/cfwl_widgetmgr.h"

CFWL_DateTimeEdit::CFWL_DateTimeEdit(
    const CFWL_App* app,
    std::unique_ptr<CFWL_WidgetProperties> properties,
    CFWL_Widget* pOuter)
    : CFWL_Edit(app, std::move(properties), pOuter) {}

void CFWL_DateTimeEdit::OnProcessMessage(CFWL_Message* pMessage) {
  if (m_pWidgetMgr->IsFormDisabled()) {
    DisForm_OnProcessMessage(pMessage);
    return;
  }

  CFWL_MessageType dwHashCode = pMessage->GetClassID();
  if (dwHashCode == CFWL_MessageType::SetFocus ||
      dwHashCode == CFWL_MessageType::KillFocus) {
    CFWL_Widget* pOuter = GetOuter();
    pOuter->GetDelegate()->OnProcessMessage(pMessage);
  }
}

void CFWL_DateTimeEdit::DisForm_OnProcessMessage(CFWL_Message* pMessage) {
  CFWL_MessageType dwHashCode = pMessage->GetClassID();
  if (!m_pWidgetMgr->IsFormDisabled() ||
      dwHashCode != CFWL_MessageType::Mouse) {
    CFWL_Edit::OnProcessMessage(pMessage);
    return;
  }

  CFWL_MsgMouse* pMouse = static_cast<CFWL_MsgMouse*>(pMessage);
  if (pMouse->m_dwCmd == FWL_MouseCommand::LeftButtonDown ||
      pMouse->m_dwCmd == FWL_MouseCommand::RightButtonDown) {
    if ((m_pProperties->m_dwStates & FWL_WGTSTATE_Focused) == 0)
      m_pProperties->m_dwStates |= FWL_WGTSTATE_Focused;

    CFWL_DateTimePicker* pDateTime =
        static_cast<CFWL_DateTimePicker*>(m_pOuter);
    if (pDateTime->IsMonthCalendarVisible()) {
      CFX_RectF rtInvalidate;
      pDateTime->GetWidgetRect(rtInvalidate);
      pDateTime->ShowMonthCalendar(false);
      rtInvalidate.Offset(-rtInvalidate.left, -rtInvalidate.top);
      pDateTime->Repaint(&rtInvalidate);
    }
  }
  CFWL_Edit::OnProcessMessage(pMessage);
}