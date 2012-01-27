/*
 * Copyright (c) 2012 Research In Motion Limited.
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 * http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#ifndef BASICDRAWING_H_
#define BASICDRAWING_H_

#include "DemoBase.h"

class Bitmap;

class BasicDrawing : public DemoBase {
public:
    BasicDrawing(SkCanvas *c, int w, int h);
    ~BasicDrawing();

    virtual void draw();

private:
    SkPaint shapePaint;
    SkPaint textPaint;
    SkRect rect;
    SkPath linPath;
    SkPath cubicPath;
    SkBitmap *bitmap;

    static const SkScalar hSpacing = 50;
    static const SkScalar vSpacing = 50;
    static const SkScalar rowHeight = 120;
    static const SkScalar colWidth = 150;

    static const char *textStrings[];
};


#endif /* BASICDRAWING_H_ */
