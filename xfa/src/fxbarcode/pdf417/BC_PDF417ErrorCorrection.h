// Copyright 2014 PDFium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#ifndef XFA_SRC_FXBARCODE_PDF417_BC_PDF417ERRORCORRECTION_H_
#define XFA_SRC_FXBARCODE_PDF417_BC_PDF417ERRORCORRECTION_H_

#include <stdint.h>

#include "core/include/fxcrt/fx_string.h"

class CBC_PDF417ErrorCorrection {
 public:
  CBC_PDF417ErrorCorrection();
  virtual ~CBC_PDF417ErrorCorrection();
  static int32_t getErrorCorrectionCodewordCount(int32_t errorCorrectionLevel,
                                                 int32_t& e);
  static int32_t getRecommendedMinimumErrorCorrectionLevel(int32_t n,
                                                           int32_t& e);
  static CFX_WideString generateErrorCorrection(CFX_WideString dataCodewords,
                                                int32_t errorCorrectionLevel,
                                                int32_t& e);

 private:
  static int32_t EC_COEFFICIENTS[][2500];
};

#endif  // XFA_SRC_FXBARCODE_PDF417_BC_PDF417ERRORCORRECTION_H_