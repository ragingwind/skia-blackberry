#include "SkDumpCanvasM.h"
#include "SkPicture.h"
#include "SkPixelRef.h"
#include "SkString.h"
#include <stdarg.h>

// needed just to know that these are all subclassed from SkFlattenable
#include "SkShader.h"
#include "SkPathEffect.h"
#include "SkXfermode.h"
#include "SkColorFilter.h"
#include "SkPathEffect.h"
#include "SkMaskFilter.h"
#include "SkEvent.h"
static void toString(const SkRect& r, SkString* str) {
    str->printf("[%g,%g %g:%g]",
                SkScalarToFloat(r.fLeft), SkScalarToFloat(r.fTop),
                SkScalarToFloat(r.width()), SkScalarToFloat(r.height()));
}

static void toString(const SkIRect& r, SkString* str) {
    str->printf("[%d,%d %d:%d]", r.fLeft, r.fTop, r.width(), r.height());
}

static void dumpVerbs(const SkPath& path, SkString* str) {
    SkPath::Iter iter(path, false);
    SkPoint pts[4];
    for (;;) {
        switch (iter.next(pts)) {
            case SkPath::kMove_Verb:
                str->appendf(" M%g,%g", pts[0].fX, pts[0].fY);
                break;
            case SkPath::kLine_Verb:
                str->appendf(" L%g,%g", pts[0].fX, pts[0].fY);
                break;
            case SkPath::kQuad_Verb:
                str->appendf(" Q%g,%g,%g,%g", pts[1].fX, pts[1].fY,
                             pts[2].fX, pts[2].fY);
                break;
            case SkPath::kCubic_Verb:
                str->appendf(" C%g,%g,%g,%g,%g,%g", pts[1].fX, pts[1].fY,
                             pts[2].fX, pts[2].fY, pts[3].fX, pts[3].fY);
                break;
            case SkPath::kClose_Verb:
                str->appendf("X");
                break;
            case SkPath::kDone_Verb:
                return;
        }
    }
}

static void toString(const SkPath& path, SkString* str) {
    if (path.isEmpty()) {
        str->set("path:empty");
    } else {
        toString(path.getBounds(), str);
#if 1
        SkString s;
        dumpVerbs(path, &s);
        str->append(s.c_str());
#endif
        str->append("]");
        str->prepend("path:[");
    }
}

static const char* toString(SkRegion::Op op) {
    static const char* gOpNames[] = {
        "DIFF", "SECT", "UNION", "XOR", "RDIFF", "REPLACE"
    };
    return gOpNames[op];
}

static void toString(const SkRegion& rgn, SkString* str) {
    toString(rgn.getBounds(), str);
    str->prepend("Region:[");
    str->append("]");
    if (rgn.isComplex()) {
        str->append(".complex");
    }
}

static const char* toString(SkCanvas::VertexMode vm) {
    static const char* gVMNames[] = {
        "TRIANGLES", "STRIP", "FAN"
    };
    return gVMNames[vm];
}

static const char* toString(SkCanvas::PointMode pm) {
    static const char* gPMNames[] = {
        "POINTS", "LINES", "POLYGON"
    };
    return gPMNames[pm];
}

static const char* toString(SkBitmap::Config config) {
    static const char* gConfigNames[] = {
        "NONE", "A1", "A8", "INDEX8", "565", "4444", "8888", "RLE"
    };
    return gConfigNames[config];
}

static void toString(const SkBitmap& bm, SkString* str) {
    str->printf("bitmap:[%d %d] %s", bm.width(), bm.height(),
                toString(bm.config()));

    SkPixelRef* pr = bm.pixelRef();
    if (NULL == pr) {
        // show null or the explicit pixel address (rare)
        str->appendf(" pixels:%p", bm.getPixels());
    } else {
        const char* uri = pr->getURI();
        if (uri) {
            str->appendf(" uri:\"%s\"", uri);
        } else {
            str->appendf(" pixelref:%p", pr);
        }
    }
}

static void toString(const void* text, size_t len, SkPaint::TextEncoding enc,
                     SkString* str) {
    switch (enc) {
        case SkPaint::kUTF8_TextEncoding:
            str->printf("\"%.*s\"%s", SkMax32(len, 32), text,
                        len > 32 ? "..." : "");
            break;
        case SkPaint::kUTF16_TextEncoding:
            str->printf("\"%.*S\"%s", SkMax32(len, 32), text,
                        len > 64 ? "..." : "");
            break;
        case SkPaint::kGlyphID_TextEncoding:
            str->set("<glyphs>");
            break;
    }
}

///////////////////////////////////////////////////////////////////////////////

SkDumpCanvasM::SkDumpCanvasM(Dumper* dumper) : fNestLevel(0) {
    SkSafeRef(dumper);
    fDumper = dumper;

    static const int WIDE_OPEN = 16384;
    SkBitmap emptyBitmap;

    emptyBitmap.setConfig(SkBitmap::kNo_Config, WIDE_OPEN, WIDE_OPEN);
    this->setBitmapDevice(emptyBitmap);
}

SkDumpCanvasM::~SkDumpCanvasM() {
    SkSafeUnref(fDumper);
}

void SkDumpCanvasM::dump(Verb verb, const SkPaint* paint,
                        const char format[], ...) {
    static const size_t BUFFER_SIZE = 1024;

    char    buffer[BUFFER_SIZE];
    va_list args;
    va_start(args, format);
    vsnprintf(buffer, BUFFER_SIZE, format, args);
    va_end(args);

    if (fDumper) {
        fDumper->dump(this, verb, buffer, paint);
    }
}

///////////////////////////////////////////////////////////////////////////////

int SkDumpCanvasM::save(SaveFlags flags) {
    this->dump(kSave_Verb, NULL, "save(0x%X)", flags);
    return this->INHERITED::save(flags);
}

int SkDumpCanvasM::saveLayer(const SkRect* bounds, const SkPaint* paint,
                             SaveFlags flags) {
    this->dump(kSave_Verb, paint, "saveLayer(0x%X)", flags);
    return this->INHERITED::saveLayer(bounds, paint, flags);
}

void SkDumpCanvasM::restore() {
    this->INHERITED::restore();
    this->dump(kRestore_Verb, NULL, "restore");
}

bool SkDumpCanvasM::translate(SkScalar dx, SkScalar dy) {
    this->dump(kMatrix_Verb, NULL, "translate(%g %g)",
               SkScalarToFloat(dx), SkScalarToFloat(dy));
    return this->INHERITED::translate(dx, dy);
}

bool SkDumpCanvasM::scale(SkScalar sx, SkScalar sy) {
    this->dump(kMatrix_Verb, NULL, "scale(%g %g)",
               SkScalarToFloat(sx), SkScalarToFloat(sy));
    return this->INHERITED::scale(sx, sy);
}

bool SkDumpCanvasM::rotate(SkScalar degrees) {
    this->dump(kMatrix_Verb, NULL, "rotate(%g)", SkScalarToFloat(degrees));
    return this->INHERITED::rotate(degrees);
}

bool SkDumpCanvasM::skew(SkScalar sx, SkScalar sy) {
    this->dump(kMatrix_Verb, NULL, "skew(%g %g)",
               SkScalarToFloat(sx), SkScalarToFloat(sy));
    return this->INHERITED::skew(sx, sy);
}

bool SkDumpCanvasM::concat(const SkMatrix& matrix) {
    SkString str;
    matrix.toDumpString(&str);
    this->dump(kMatrix_Verb, NULL, "concat(%s)", str.c_str());
    return this->INHERITED::concat(matrix);
}

void SkDumpCanvasM::setMatrix(const SkMatrix& matrix) {
    SkString str;
    matrix.toDumpString(&str);
    this->dump(kMatrix_Verb, NULL, "setMatrix(%s)", str.c_str());
    this->INHERITED::setMatrix(matrix);
}

///////////////////////////////////////////////////////////////////////////////

bool SkDumpCanvasM::clipRect(const SkRect& rect, SkRegion::Op op) {
    SkString str;
    toString(rect, &str);
    this->dump(kClip_Verb, NULL, "clipRect(%s %s)", str.c_str(), toString(op));
    return this->INHERITED::clipRect(rect, op);
}

bool SkDumpCanvasM::clipPath(const SkPath& path, SkRegion::Op op) {
    SkString str;
    toString(path, &str);
    this->dump(kClip_Verb, NULL, "clipPath(%s %s)", str.c_str(), toString(op));
    return this->INHERITED::clipPath(path, op);
}

bool SkDumpCanvasM::clipRegion(const SkRegion& deviceRgn, SkRegion::Op op) {
    SkString str;
    toString(deviceRgn, &str);
    this->dump(kClip_Verb, NULL, "clipRegion(%s %s)", str.c_str(),
               toString(op));
    return this->INHERITED::clipRegion(deviceRgn, op);
}

///////////////////////////////////////////////////////////////////////////////

void SkDumpCanvasM::drawPaint(const SkPaint& paint) {
    this->dump(kDrawPaint_Verb, &paint, "drawPaint()");
}

void SkDumpCanvasM::drawPoints(PointMode mode, size_t count,
                               const SkPoint pts[], const SkPaint& paint) {
    this->dump(kDrawPoints_Verb, &paint, "drawPoints(%s, %d)", toString(mode),
               count);
}

void SkDumpCanvasM::drawRect(const SkRect& rect, const SkPaint& paint) {
    SkString str;
    toString(rect, &str);
    this->dump(kDrawRect_Verb, &paint, "drawRect(%s)", str.c_str());
}

void SkDumpCanvasM::drawPath(const SkPath& path, const SkPaint& paint) {
    SkString str;
    toString(path, &str);
    this->dump(kDrawPath_Verb, &paint, "drawPath(%s)", str.c_str());
}

void SkDumpCanvasM::drawBitmap(const SkBitmap& bitmap, SkScalar x, SkScalar y,
                               const SkPaint* paint) {
    SkString str;
    toString(bitmap, &str);
    this->dump(kDrawBitmap_Verb, paint, "drawBitmap(%s %g %g)", str.c_str(),
               SkScalarToFloat(x), SkScalarToFloat(y));
}

void SkDumpCanvasM::drawBitmapRect(const SkBitmap& bitmap, const SkIRect* src,
                                   const SkRect& dst, const SkPaint* paint) {
    SkString bs, rs;
    toString(bitmap, &bs);
    toString(dst, &rs);
    // show the src-rect only if its not everything
    if (src && (src->fLeft > 0 || src->fTop > 0 ||
                src->fRight < bitmap.width() ||
                src->fBottom < bitmap.height())) {
        SkString ss;
        toString(*src, &ss);
        rs.prependf("%s ", ss.c_str());
    }

    this->dump(kDrawBitmap_Verb, paint, "drawBitmapRect(%s %s)",
               bs.c_str(), rs.c_str());
}

void SkDumpCanvasM::drawBitmapMatrix(const SkBitmap& bitmap, const SkMatrix& m,
                                     const SkPaint* paint) {
    SkString bs, ms;
    toString(bitmap, &bs);
    m.toDumpString(&ms);
    this->dump(kDrawBitmap_Verb, paint, "drawBitmapMatrix(%s %s)",
               bs.c_str(), ms.c_str());
}

void SkDumpCanvasM::drawSprite(const SkBitmap& bitmap, int x, int y,
                               const SkPaint* paint) {
    SkString str;
    toString(bitmap, &str);
    this->dump(kDrawBitmap_Verb, paint, "drawSprite(%s %d %d)", str.c_str(),
               x, y);
}

void SkDumpCanvasM::drawText(const void* text, size_t byteLength, SkScalar x,
                             SkScalar y, const SkPaint& paint) {
    SkString str;
    toString(text, byteLength, paint.getTextEncoding(), &str);
    this->dump(kDrawText_Verb, &paint, "drawText(%s [%d] %g %g)", str.c_str(),
               byteLength, SkScalarToFloat(x), SkScalarToFloat(y));
}

void SkDumpCanvasM::drawPosText(const void* text, size_t byteLength,
                                const SkPoint pos[], const SkPaint& paint) {
    SkString str;
    toString(text, byteLength, paint.getTextEncoding(), &str);
    this->dump(kDrawText_Verb, &paint, "drawPosText(%s [%d] %g %g ...)",
               str.c_str(), byteLength, SkScalarToFloat(pos[0].fX),
               SkScalarToFloat(pos[0].fY));
}

void SkDumpCanvasM::drawPosTextH(const void* text, size_t byteLength,
                                 const SkScalar xpos[], SkScalar constY,
                                 const SkPaint& paint) {
    SkString str;
    toString(text, byteLength, paint.getTextEncoding(), &str);
    this->dump(kDrawText_Verb, &paint, "drawPosTextH(%s [%d] %g %g ...)",
               str.c_str(), byteLength, SkScalarToFloat(xpos[0]),
               SkScalarToFloat(constY));
}

void SkDumpCanvasM::drawTextOnPath(const void* text, size_t byteLength,
                                   const SkPath& path, const SkMatrix* matrix,
                                   const SkPaint& paint) {
    SkString str;
    toString(text, byteLength, paint.getTextEncoding(), &str);
    this->dump(kDrawText_Verb, &paint, "drawTextOnPath(%s [%d])",
               str.c_str(), byteLength);
}

void SkDumpCanvasM::drawPicture(SkPicture& picture) {
    this->dump(kDrawPicture_Verb, NULL, "drawPicture(%p) %d:%d", &picture,
               picture.width(), picture.height());
    fNestLevel += 1;
    this->INHERITED::drawPicture(picture);
    fNestLevel -= 1;
    this->dump(kDrawPicture_Verb, NULL, "endPicture(%p) %d:%d", &picture,
               picture.width(), picture.height());
}

void SkDumpCanvasM::drawVertices(VertexMode vmode, int vertexCount,
                                 const SkPoint vertices[], const SkPoint texs[],
                                 const SkColor colors[], SkXfermode* xmode,
                                 const uint16_t indices[], int indexCount,
                                 const SkPaint& paint) {
    this->dump(kDrawVertices_Verb, &paint, "drawVertices(%s [%d] %g %g ...)",
               toString(vmode), vertexCount, SkScalarToFloat(vertices[0].fX),
               SkScalarToFloat(vertices[0].fY));
}

void SkDumpCanvasM::drawData(const void* data, size_t length) {
//    this->dump(kDrawData_Verb, NULL, "drawData(%d)", length);
    this->dump(kDrawData_Verb, NULL, "drawData(%d) %.*s", length,
               SkMin32(length, 64), data);
}