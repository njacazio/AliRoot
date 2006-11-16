// $Header$

#ifndef REVE_RGBAPalette_H
#define REVE_RGBAPalette_H

#include <Reve/Reve.h>

#include <TObject.h>

namespace Reve {

class RGBAPalette : public TObject, public ReferenceCount
{
  friend class RGBAPaletteEditor;
  friend class RGBAPaletteSubEditor;

public:
  enum LimitAction_e { LA_Cut, LA_Mark, LA_Clip, LA_Wrap };

private:
  RGBAPalette(const RGBAPalette&);            // Not implemented
  RGBAPalette& operator=(const RGBAPalette&); // Not implemented

protected:
  Int_t     fLowLimit;  // Low  limit for Min/Max values (used by editor)
  Int_t     fHighLimit; // High limit for Min/Max values (used by editor)
  Int_t     fMinVal;
  Int_t     fMaxVal;
  Int_t     fNBins;

  Bool_t        fInterpolate;
  Bool_t        fShowDefValue;
  Int_t fUndershootAction;
  Int_t fOvershootAction;

  Color_t   fDefaultColor;   // Color for when value is not specified
  UChar_t   fDefaultRGBA[4];
  Color_t   fUnderColor;     // Undershoot color
  UChar_t   fUnderRGBA[4];
  Color_t   fOverColor;      // Overshoot color
  UChar_t   fOverRGBA[4];

  mutable UChar_t* fColorArray; //[4*fNBins]

  void SetupColor(Int_t val, UChar_t* pix) const;

  static RGBAPalette* fgDefaultPalette;

public:
  RGBAPalette();
  RGBAPalette(Int_t min, Int_t max);
  RGBAPalette(Int_t min, Int_t max, Bool_t interp, Bool_t showdef=kTRUE);
  virtual ~RGBAPalette();

  void SetupColorArray() const;
  void ClearColorArray();

  Bool_t   WithinVisibleRange(Int_t val) const;
  const UChar_t* ColorFromValue(Int_t val) const;
  void     ColorFromValue(Int_t val, UChar_t* pix, Bool_t alpha=kTRUE) const;
  Bool_t   ColorFromValue(Int_t val, Int_t defVal, UChar_t* pix, Bool_t alpha=kTRUE) const;

  Int_t  GetMinVal() const { return fMinVal; }
  Int_t  GetMaxVal() const { return fMaxVal; }

  void   SetLimits(Int_t low, Int_t high);
  void   SetMinMax(Int_t min, Int_t max);
  void   SetMin(Int_t min);
  void   SetMax(Int_t max);

  // ================================================================

  Bool_t GetInterpolate() const { return fInterpolate; }
  void   SetInterpolate(Bool_t b);

  Bool_t GetShowDefValue() const { return fShowDefValue; }
  void   SetShowDefValue(Bool_t v) { fShowDefValue = v; }

  Int_t GetUndershootAction() const  { return fUndershootAction; }
  Int_t GetOvershootAction()  const  { return fOvershootAction;  }
  void  SetUndershootAction(Int_t a) { fUndershootAction = a;    }
  void  SetOvershootAction(Int_t a)  { fOvershootAction  = a;    }

  // ================================================================

  Color_t  GetDefaultColor() const { return fDefaultColor; }
  Color_t* PtrDefaultColor() { return &fDefaultColor; }
  UChar_t* GetDefaultRGBA()  { return fDefaultRGBA;  }
  const UChar_t* GetDefaultRGBA() const { return fDefaultRGBA;  }

  void   SetDefaultColor(Color_t ci);
  void   SetDefaultColor(Pixel_t pix);
  void   SetDefaultColor(UChar_t r, UChar_t g, UChar_t b, UChar_t a=255);

  // ----------------------------------------------------------------

  Color_t  GetUnderColor() const { return fUnderColor; }
  Color_t* PtrUnderColor() { return &fUnderColor; }
  UChar_t* GetUnderRGBA()  { return fUnderRGBA;  }
  const UChar_t* GetUnderRGBA() const { return fUnderRGBA;  }

  void   SetUnderColor(Color_t ci);
  void   SetUnderColor(Pixel_t pix);
  void   SetUnderColor(UChar_t r, UChar_t g, UChar_t b, UChar_t a=255);

  // ----------------------------------------------------------------

  Color_t  GetOverColor() const { return fOverColor; }
  Color_t* PtrOverColor() { return &fOverColor; }
  UChar_t* GetOverRGBA()  { return fOverRGBA;  }
  const UChar_t* GetOverRGBA() const { return fOverRGBA;  }

  void   SetOverColor(Color_t ci);
  void   SetOverColor(Pixel_t pix);
  void   SetOverColor(UChar_t r, UChar_t g, UChar_t b, UChar_t a=255);

  // ================================================================

  // ?? Should we emit some *SIGNALS* ??
  // ?? Should we have a RendererTimeStamp ??

  ClassDef(RGBAPalette, 1);
}; // endclass RGBAPalette


/**************************************************************************/
// Inlines for RGBAPalette
/**************************************************************************/

inline Bool_t RGBAPalette::WithinVisibleRange(Int_t val) const
{
  if ((val < fMinVal && fUndershootAction == LA_Cut) ||
      (val > fMaxVal && fOvershootAction  == LA_Cut))
    return kFALSE;
  else
    return kTRUE;
}

inline const UChar_t* RGBAPalette::ColorFromValue(Int_t val) const
{
  // Here we expect that LA_Cut has been checked; we further check
  // for LA_Wrap and LA_Clip otherwise we proceed as for LA_Mark.

  if (!fColorArray)  SetupColorArray();
  if (val < fMinVal) {
    if (fUndershootAction == LA_Wrap)
      val = (val+1-fMinVal)%fNBins + fMaxVal;
    else if (fUndershootAction == LA_Clip)
      val = fMinVal;
    else
      return fUnderRGBA;
  }
  else if(val > fMaxVal) {
    if (fOvershootAction == LA_Wrap)
      val = (val-1-fMaxVal)%fNBins + fMinVal;
    else if (fOvershootAction == LA_Clip)
      val = fMaxVal;
    else
      return fOverRGBA;
  }
  return fColorArray + 4 * (val - fMinVal);
}

inline void RGBAPalette::ColorFromValue(Int_t val, UChar_t* pix, Bool_t alpha) const
{
  const UChar_t* c = ColorFromValue(val);
  pix[0] = c[0]; pix[1] = c[1]; pix[2] = c[2];
  if (alpha) pix[3] = c[3];
}

inline Bool_t RGBAPalette::ColorFromValue(Int_t val, Int_t defVal, UChar_t* pix, Bool_t alpha) const
{
  if (val == defVal) {
    if (fShowDefValue) {
      pix[0] = fDefaultRGBA[0];
      pix[1] = fDefaultRGBA[1];
      pix[2] = fDefaultRGBA[2];
      if (alpha) pix[3] = fDefaultRGBA[3]; 
      return kTRUE;
    } else {
      return kFALSE;
    }
  }

  if (WithinVisibleRange(val)) {
    ColorFromValue(val, pix, alpha);
    return kTRUE;
  } else {
    return kFALSE;
  }
}

}

#endif
