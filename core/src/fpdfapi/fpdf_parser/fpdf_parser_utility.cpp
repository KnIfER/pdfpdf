// Copyright 2014 PDFium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#include "core/include/fpdfapi/fpdf_parser.h"

#include "core/include/fxcrt/fx_ext.h"

// Indexed by 8-bit character code, contains either:
//   'W' - for whitespace: NUL, TAB, CR, LF, FF, SPACE, 0x80, 0xff
//   'N' - for numeric: 0123456789+-.
//   'D' - for delimiter: %()/<>[]{}
//   'R' - otherwise.
const char PDF_CharType[256] = {
    // NUL  SOH  STX  ETX  EOT  ENQ  ACK  BEL  BS   HT   LF   VT   FF   CR   SO
    // SI
    'W', 'R', 'R', 'R', 'R', 'R', 'R', 'R', 'R', 'W', 'W', 'R', 'W', 'W', 'R',
    'R',

    // DLE  DC1  DC2  DC3  DC4  NAK  SYN  ETB  CAN  EM   SUB  ESC  FS   GS   RS
    // US
    'R', 'R', 'R', 'R', 'R', 'R', 'R', 'R', 'R', 'R', 'R', 'R', 'R', 'R', 'R',
    'R',

    // SP    !    "    #    $    %    &    ´    (    )    *    +    ,    -    .
    // /
    'W', 'R', 'R', 'R', 'R', 'D', 'R', 'R', 'D', 'D', 'R', 'N', 'R', 'N', 'N',
    'D',

    // 0    1    2    3    4    5    6    7    8    9    :    ;    <    =    > ?
    'N', 'N', 'N', 'N', 'N', 'N', 'N', 'N', 'N', 'N', 'R', 'R', 'D', 'R', 'D',
    'R',

    // @    A    B    C    D    E    F    G    H    I    J    K    L    M    N O
    'R', 'R', 'R', 'R', 'R', 'R', 'R', 'R', 'R', 'R', 'R', 'R', 'R', 'R', 'R',
    'R',

    // P    Q    R    S    T    U    V    W    X    Y    Z    [    \    ]    ^ _
    'R', 'R', 'R', 'R', 'R', 'R', 'R', 'R', 'R', 'R', 'R', 'D', 'R', 'D', 'R',
    'R',

    // `    a    b    c    d    e    f    g    h    i    j    k    l    m    n o
    'R', 'R', 'R', 'R', 'R', 'R', 'R', 'R', 'R', 'R', 'R', 'R', 'R', 'R', 'R',
    'R',

    // p    q    r    s    t    u    v    w    x    y    z    {    |    }    ~
    // DEL
    'R', 'R', 'R', 'R', 'R', 'R', 'R', 'R', 'R', 'R', 'R', 'D', 'R', 'D', 'R',
    'R',

    'W', 'R', 'R', 'R', 'R', 'R', 'R', 'R', 'R', 'R', 'R', 'R', 'R', 'R', 'R',
    'R', 'R', 'R', 'R', 'R', 'R', 'R', 'R', 'R', 'R', 'R', 'R', 'R', 'R', 'R',
    'R', 'R', 'R', 'R', 'R', 'R', 'R', 'R', 'R', 'R', 'R', 'R', 'R', 'R', 'R',
    'R', 'R', 'R', 'R', 'R', 'R', 'R', 'R', 'R', 'R', 'R', 'R', 'R', 'R', 'R',
    'R', 'R', 'R', 'R', 'R', 'R', 'R', 'R', 'R', 'R', 'R', 'R', 'R', 'R', 'R',
    'R', 'R', 'R', 'R', 'R', 'R', 'R', 'R', 'R', 'R', 'R', 'R', 'R', 'R', 'R',
    'R', 'R', 'R', 'R', 'R', 'R', 'R', 'R', 'R', 'R', 'R', 'R', 'R', 'R', 'R',
    'R', 'R', 'R', 'R', 'R', 'R', 'R', 'R', 'R', 'R', 'R', 'R', 'R', 'R', 'R',
    'R', 'R', 'R', 'R', 'R', 'R', 'R', 'W'};

CPDF_SimpleParser::CPDF_SimpleParser(const uint8_t* pData, FX_DWORD dwSize) {
  m_pData = pData;
  m_dwSize = dwSize;
  m_dwCurPos = 0;
}

CPDF_SimpleParser::CPDF_SimpleParser(const CFX_ByteStringC& str) {
  m_pData = str.GetPtr();
  m_dwSize = str.GetLength();
  m_dwCurPos = 0;
}

void CPDF_SimpleParser::ParseWord(const uint8_t*& pStart, FX_DWORD& dwSize) {
  pStart = NULL;
  dwSize = 0;
  uint8_t ch;
  while (1) {
    if (m_dwSize <= m_dwCurPos)
      return;
    ch = m_pData[m_dwCurPos++];
    while (PDFCharIsWhitespace(ch)) {
      if (m_dwSize <= m_dwCurPos)
        return;
      ch = m_pData[m_dwCurPos++];
    }

    if (ch != '%')
      break;

    while (1) {
      if (m_dwSize <= m_dwCurPos)
        return;
      ch = m_pData[m_dwCurPos++];
      if (ch == '\r' || ch == '\n')
        break;
    }
  }

  FX_DWORD start_pos = m_dwCurPos - 1;
  pStart = m_pData + start_pos;
  if (PDFCharIsDelimiter(ch)) {
    if (ch == '/') {
      while (1) {
        if (m_dwSize <= m_dwCurPos)
          return;
        ch = m_pData[m_dwCurPos++];
        if (!PDFCharIsOther(ch) && !PDFCharIsNumeric(ch)) {
          m_dwCurPos--;
          dwSize = m_dwCurPos - start_pos;
          return;
        }
      }
    } else {
      dwSize = 1;
      if (ch == '<') {
        if (m_dwSize <= m_dwCurPos)
          return;
        ch = m_pData[m_dwCurPos++];
        if (ch == '<')
          dwSize = 2;
        else
          m_dwCurPos--;
      } else if (ch == '>') {
        if (m_dwSize <= m_dwCurPos)
          return;
        ch = m_pData[m_dwCurPos++];
        if (ch == '>')
          dwSize = 2;
        else
          m_dwCurPos--;
      }
    }
    return;
  }

  dwSize = 1;
  while (1) {
    if (m_dwSize <= m_dwCurPos)
      return;
    ch = m_pData[m_dwCurPos++];

    if (PDFCharIsDelimiter(ch) || PDFCharIsWhitespace(ch)) {
      m_dwCurPos--;
      break;
    }
    dwSize++;
  }
}

CFX_ByteStringC CPDF_SimpleParser::GetWord() {
  const uint8_t* pStart;
  FX_DWORD dwSize;
  ParseWord(pStart, dwSize);
  if (dwSize == 1 && pStart[0] == '<') {
    while (m_dwCurPos < m_dwSize && m_pData[m_dwCurPos] != '>') {
      m_dwCurPos++;
    }
    if (m_dwCurPos < m_dwSize) {
      m_dwCurPos++;
    }
    return CFX_ByteStringC(pStart,
                           (FX_STRSIZE)(m_dwCurPos - (pStart - m_pData)));
  }
  if (dwSize == 1 && pStart[0] == '(') {
    int level = 1;
    while (m_dwCurPos < m_dwSize) {
      if (m_pData[m_dwCurPos] == ')') {
        level--;
        if (level == 0) {
          break;
        }
      }
      if (m_pData[m_dwCurPos] == '\\') {
        if (m_dwSize <= m_dwCurPos) {
          break;
        }
        m_dwCurPos++;
      } else if (m_pData[m_dwCurPos] == '(') {
        level++;
      }
      if (m_dwSize <= m_dwCurPos) {
        break;
      }
      m_dwCurPos++;
    }
    if (m_dwCurPos < m_dwSize) {
      m_dwCurPos++;
    }
    return CFX_ByteStringC(pStart,
                           (FX_STRSIZE)(m_dwCurPos - (pStart - m_pData)));
  }
  return CFX_ByteStringC(pStart, dwSize);
}

bool CPDF_SimpleParser::FindTagParamFromStart(const CFX_ByteStringC& token,
                                              int nParams) {
  nParams++;
  FX_DWORD* pBuf = FX_Alloc(FX_DWORD, nParams);
  int buf_index = 0;
  int buf_count = 0;
  m_dwCurPos = 0;
  while (1) {
    pBuf[buf_index++] = m_dwCurPos;
    if (buf_index == nParams) {
      buf_index = 0;
    }
    buf_count++;
    if (buf_count > nParams) {
      buf_count = nParams;
    }
    CFX_ByteStringC word = GetWord();
    if (word.IsEmpty()) {
      FX_Free(pBuf);
      return false;
    }
    if (word == token) {
      if (buf_count < nParams) {
        continue;
      }
      m_dwCurPos = pBuf[buf_index];
      FX_Free(pBuf);
      return true;
    }
  }
  return false;
}

CFX_ByteString PDF_NameDecode(const CFX_ByteStringC& bstr) {
  int size = bstr.GetLength();
  const FX_CHAR* pSrc = bstr.GetCStr();
  if (!FXSYS_memchr(pSrc, '#', size)) {
    return bstr;
  }
  CFX_ByteString result;
  FX_CHAR* pDestStart = result.GetBuffer(size);
  FX_CHAR* pDest = pDestStart;
  for (int i = 0; i < size; i++) {
    if (pSrc[i] == '#' && i < size - 2) {
      *pDest++ =
          FXSYS_toHexDigit(pSrc[i + 1]) * 16 + FXSYS_toHexDigit(pSrc[i + 2]);
      i += 2;
    } else {
      *pDest++ = pSrc[i];
    }
  }
  result.ReleaseBuffer((FX_STRSIZE)(pDest - pDestStart));
  return result;
}

CFX_ByteString PDF_NameDecode(const CFX_ByteString& orig) {
  if (!FXSYS_memchr(orig.c_str(), '#', orig.GetLength())) {
    return orig;
  }
  return PDF_NameDecode(CFX_ByteStringC(orig));
}

CFX_ByteString PDF_NameEncode(const CFX_ByteString& orig) {
  uint8_t* src_buf = (uint8_t*)orig.c_str();
  int src_len = orig.GetLength();
  int dest_len = 0;
  int i;
  for (i = 0; i < src_len; i++) {
    uint8_t ch = src_buf[i];
    if (ch >= 0x80 || PDFCharIsWhitespace(ch) || ch == '#' ||
        PDFCharIsDelimiter(ch)) {
      dest_len += 3;
    } else {
      dest_len++;
    }
  }
  if (dest_len == src_len)
    return orig;

  CFX_ByteString res;
  FX_CHAR* dest_buf = res.GetBuffer(dest_len);
  dest_len = 0;
  for (i = 0; i < src_len; i++) {
    uint8_t ch = src_buf[i];
    if (ch >= 0x80 || PDFCharIsWhitespace(ch) || ch == '#' ||
        PDFCharIsDelimiter(ch)) {
      dest_buf[dest_len++] = '#';
      dest_buf[dest_len++] = "0123456789ABCDEF"[ch / 16];
      dest_buf[dest_len++] = "0123456789ABCDEF"[ch % 16];
    } else {
      dest_buf[dest_len++] = ch;
    }
  }
  dest_buf[dest_len] = 0;
  res.ReleaseBuffer();
  return res;
}

CFX_ByteTextBuf& operator<<(CFX_ByteTextBuf& buf, const CPDF_Object* pObj) {
  if (!pObj) {
    buf << " null";
    return buf;
  }
  switch (pObj->GetType()) {
    case CPDF_Object::NULLOBJ:
      buf << " null";
      break;
    case CPDF_Object::BOOLEAN:
    case CPDF_Object::NUMBER:
      buf << " " << pObj->GetString();
      break;
    case CPDF_Object::STRING:
      buf << PDF_EncodeString(pObj->GetString(), pObj->AsString()->IsHex());
      break;
    case CPDF_Object::NAME: {
      CFX_ByteString str = pObj->GetString();
      buf << "/" << PDF_NameEncode(str);
      break;
    }
    case CPDF_Object::REFERENCE: {
      buf << " " << pObj->AsReference()->GetRefObjNum() << " 0 R ";
      break;
    }
    case CPDF_Object::ARRAY: {
      const CPDF_Array* p = pObj->AsArray();
      buf << "[";
      for (FX_DWORD i = 0; i < p->GetCount(); i++) {
        CPDF_Object* pElement = p->GetElement(i);
        if (pElement->GetObjNum()) {
          buf << " " << pElement->GetObjNum() << " 0 R";
        } else {
          buf << pElement;
        }
      }
      buf << "]";
      break;
    }
    case CPDF_Object::DICTIONARY: {
      const CPDF_Dictionary* p = pObj->AsDictionary();
      buf << "<<";
      for (const auto& it : *p) {
        const CFX_ByteString& key = it.first;
        CPDF_Object* pValue = it.second;
        buf << "/" << PDF_NameEncode(key);
        if (pValue && pValue->GetObjNum()) {
          buf << " " << pValue->GetObjNum() << " 0 R ";
        } else {
          buf << pValue;
        }
      }
      buf << ">>";
      break;
    }
    case CPDF_Object::STREAM: {
      const CPDF_Stream* p = pObj->AsStream();
      buf << p->GetDict() << "stream\r\n";
      CPDF_StreamAcc acc;
      acc.LoadAllData(p, TRUE);
      buf.AppendBlock(acc.GetData(), acc.GetSize());
      buf << "\r\nendstream";
      break;
    }
    default:
      ASSERT(FALSE);
      break;
  }
  return buf;
}

FX_FLOAT PDF_ClipFloat(FX_FLOAT f) {
  if (f < 0) {
    return 0;
  }
  if (f > 1.0f) {
    return 1.0f;
  }
  return f;
}

static CPDF_Object* SearchNumberNode(CPDF_Dictionary* pNode, int num) {
  CPDF_Array* pLimits = pNode->GetArrayBy("Limits");
  if (pLimits &&
      (num < pLimits->GetIntegerAt(0) || num > pLimits->GetIntegerAt(1))) {
    return NULL;
  }
  CPDF_Array* pNumbers = pNode->GetArrayBy("Nums");
  if (pNumbers) {
    FX_DWORD dwCount = pNumbers->GetCount() / 2;
    for (FX_DWORD i = 0; i < dwCount; i++) {
      int index = pNumbers->GetIntegerAt(i * 2);
      if (num == index) {
        return pNumbers->GetElementValue(i * 2 + 1);
      }
      if (index > num) {
        break;
      }
    }
    return NULL;
  }
  CPDF_Array* pKids = pNode->GetArrayBy("Kids");
  if (!pKids) {
    return NULL;
  }
  for (FX_DWORD i = 0; i < pKids->GetCount(); i++) {
    CPDF_Dictionary* pKid = pKids->GetDictAt(i);
    if (!pKid) {
      continue;
    }
    CPDF_Object* pFound = SearchNumberNode(pKid, num);
    if (pFound) {
      return pFound;
    }
  }
  return NULL;
}

CPDF_Object* CPDF_NumberTree::LookupValue(int num) {
  return SearchNumberNode(m_pRoot, num);
}