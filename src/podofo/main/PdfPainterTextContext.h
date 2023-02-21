/**
 * SPDX-FileCopyrightText: (C) 2005 Dominik Seichter <domseichter@web.de>
 * SPDX-FileCopyrightText: (C) 2020 Francesco Pretto <ceztko@gmail.com>
 * SPDX-License-Identifier: LGPL-2.0-or-later
 */

#ifndef PDF_PAINTER_PATH_H
#define PDF_PAINTER_PATH_H

#include "PdfStringStream.h"

namespace PoDoFo {

enum class PdfDrawTextStyle
{
    Regular = 0,
    StrikeOut = 1,
    Underline = 2,
};

struct PODOFO_API PdfDrawTextMultiLineParams final
{
    PdfDrawTextStyle Style = PdfDrawTextStyle::Regular;                         ///< style of the draw text operation
    PdfHorizontalAlignment HorizontalAlignment = PdfHorizontalAlignment::Left;  ///< alignment of the individual text lines in the given bounding box
    PdfVerticalAlignment VerticalAlignment = PdfVerticalAlignment::Top;         ///< vertical alignment of the text in the given bounding box
    bool Clip = true;                                                           ///< set the clipping rectangle to the given rect, otherwise no clipping is performed
    bool SkipSpaces = true;                                                     ///< whether the trailing whitespaces should be skipped, so that next line doesn't start with whitespace
};

/**
 * This class describes PDF paths being written to a PdfPainter
 */
class PODOFO_API PdfPainterTextContext final
{
    friend class PdfPainter;

private:
    PdfPainterTextContext(PdfPainter& painter);

public:
    /** Begin drawing multiple text strings on a page using a given font object.
     *  You have to call SetFont before calling this function.
     *
     *  If you want more simpler text output and do not need
     *  the advanced text position features of MoveTextPos
     *  use DrawText which is easier.
 
     *
     *  \see AddText()
     *  \see MoveTo()
     *  \see EndText()
     */
    void Begin();

    /** Draw a string on a page.
     *  You have to call BeginText before the first call of this function
     *  and EndText after the last call.
     *
     *  If you want more simpler text output and do not need
     *  the advanced text position features of MoveTextPos
     *  use DrawText which is easier.
     *
     *  \param str the text string which should be printed
     *
     *  \see SetFont()
     *  \see MoveTo()
     *  \see End()
     */
    void AddText(const std::string_view& str);

    /** Move position for text drawing on a page.
     *  You have to call BeginText before calling this function
     *
     *  If you want more simpler text output and do not need
     *  the advanced text position features of MoveTextPos
     *  use DrawText which is easier.
     *
     *  \param x the x offset relative to pos of BeginText or last MoveTextPos
     *  \param y the y offset relative to pos of BeginText or last MoveTextPos
     *
     *  \see Begin()
     *  \see AddText()
     *  \see End()
     */
    void MoveTo(double x, double y);

    /** End drawing multiple text strings on a page
     *
     *  If you want more simpler text output and do not need
     *  the advanced text position features of MoveTextPos
     *  use DrawText which is easier.
     *
     *  \see Begin()
     *  \see AddText()
     *  \see MoveTo()
     */
    void End();

    /** Draw a single-line text string on a page using a given font object.
     *  You have to call SetFont before calling this function.
     *  \param str the text string which should be printed
     *  \param x the x coordinate
     *  \param y the y coordinate
     */
    void DrawText(const std::string_view& str, double x, double y,
        PdfDrawTextStyle style = PdfDrawTextStyle::Regular);

    /** Draw multiline text into a rectangle doing automatic wordwrapping.
     *  The current font is used and SetFont has to be called at least once
     *  before using this function
     *
     *  \param str the text which should be drawn
     *  \param x the x coordinate of the text area (left)
     *  \param y the y coordinate of the text area (bottom)
     *  \param width width of the text area
     *  \param height height of the text area
     *  \param params parameters of the draw operation
     */
    void DrawTextMultiLine(const std::string_view& str, double x, double y, double width, double height,
        const PdfDrawTextMultiLineParams& params = { });

    /** Draw multiline text into a rectangle doing automatic wordwrapping.
     *  The current font is used and SetFont has to be called at least once
     *  before using this function
     *
     *  \param str the text which should be drawn
     *  \param rect bounding rectangle of the text
     *  \param params parameters of the draw operation
     */
    void DrawTextMultiLine(const std::string_view& str, const PdfRect& rect,
        const PdfDrawTextMultiLineParams& params = { });

    /** Draw a single line of text horizontally aligned.
     *  \param str the text to draw
     *  \param x the x coordinate of the text line
     *  \param y the y coordinate of the text line
     *  \param width the width of the text line
     *  \param hAlignment alignment of the text line
     *  \param style style of the draw text operation
     */
    void DrawTextAligned(const std::string_view& str, double x, double y,
        double width, PdfHorizontalAlignment hAlignment,
        PdfDrawTextStyle style = PdfDrawTextStyle::Regular);

private:
    PdfPainter* m_painter;
};

}

#endif // PDF_PAINTER_PATH_H
