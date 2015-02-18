
//+-----------------------------------------------------------------------------
//
//  Copyright (c) Microsoft Corporation.  All rights reserved.
//
//  $Description:
//      Sample Direct2D smart pointer header file
//
//  $ENDTAG
//
//------------------------------------------------------------------------------

#pragma once

#ifdef _D2D1_H_

_COM_SMARTPTR_TYPEDEF(ID2D1Bitmap, __uuidof(ID2D1Bitmap));
_COM_SMARTPTR_TYPEDEF(ID2D1BitmapBrush, __uuidof(ID2D1BitmapBrush));
_COM_SMARTPTR_TYPEDEF(ID2D1BitmapRenderTarget, __uuidof(ID2D1BitmapRenderTarget));
_COM_SMARTPTR_TYPEDEF(ID2D1Brush, __uuidof(ID2D1Brush));
_COM_SMARTPTR_TYPEDEF(ID2D1DCRenderTarget, __uuidof(ID2D1DCRenderTarget));
_COM_SMARTPTR_TYPEDEF(ID2D1DrawingStateBlock, __uuidof(ID2D1DrawingStateBlock));
_COM_SMARTPTR_TYPEDEF(ID2D1EllipseGeometry, __uuidof(ID2D1EllipseGeometry));
_COM_SMARTPTR_TYPEDEF(ID2D1Factory, __uuidof(ID2D1Factory));
_COM_SMARTPTR_TYPEDEF(ID2D1GdiInteropRenderTarget, __uuidof(ID2D1GdiInteropRenderTarget));
_COM_SMARTPTR_TYPEDEF(ID2D1Geometry, __uuidof(ID2D1Geometry));
_COM_SMARTPTR_TYPEDEF(ID2D1GeometryGroup, __uuidof(ID2D1GeometryGroup));
_COM_SMARTPTR_TYPEDEF(ID2D1GeometrySink, __uuidof(ID2D1GeometrySink));
_COM_SMARTPTR_TYPEDEF(ID2D1GradientStopCollection, __uuidof(ID2D1GradientStopCollection));
_COM_SMARTPTR_TYPEDEF(ID2D1HwndRenderTarget, __uuidof(ID2D1HwndRenderTarget));
_COM_SMARTPTR_TYPEDEF(ID2D1Layer, __uuidof(ID2D1Layer));
_COM_SMARTPTR_TYPEDEF(ID2D1LinearGradientBrush, __uuidof(ID2D1LinearGradientBrush));
_COM_SMARTPTR_TYPEDEF(ID2D1Mesh, __uuidof(ID2D1Mesh));
_COM_SMARTPTR_TYPEDEF(ID2D1PathGeometry, __uuidof(ID2D1PathGeometry));
_COM_SMARTPTR_TYPEDEF(ID2D1RadialGradientBrush, __uuidof(ID2D1RadialGradientBrush));
_COM_SMARTPTR_TYPEDEF(ID2D1RectangleGeometry, __uuidof(ID2D1RectangleGeometry));
_COM_SMARTPTR_TYPEDEF(ID2D1RenderTarget, __uuidof(ID2D1RenderTarget));
_COM_SMARTPTR_TYPEDEF(ID2D1Resource, __uuidof(ID2D1Resource));
_COM_SMARTPTR_TYPEDEF(ID2D1RoundedRectangleGeometry, __uuidof(ID2D1RoundedRectangleGeometry));
_COM_SMARTPTR_TYPEDEF(ID2D1SimplifiedGeometrySink, __uuidof(ID2D1SimplifiedGeometrySink));
_COM_SMARTPTR_TYPEDEF(ID2D1SolidColorBrush, __uuidof(ID2D1SolidColorBrush));
_COM_SMARTPTR_TYPEDEF(ID2D1StrokeStyle, __uuidof(ID2D1StrokeStyle));
_COM_SMARTPTR_TYPEDEF(ID2D1TessellationSink, __uuidof(ID2D1TessellationSink));
_COM_SMARTPTR_TYPEDEF(ID2D1TransformedGeometry, __uuidof(ID2D1TransformedGeometry));
//New interface
_COM_SMARTPTR_TYPEDEF(ID2D1DeviceContext, __uuidof(ID2D1DeviceContext));

#endif

#ifdef __d3d10_h__

_COM_SMARTPTR_TYPEDEF(ID3D10Asynchronous, __uuidof(ID3D10Asynchronous));
_COM_SMARTPTR_TYPEDEF(ID3D10BlendState, __uuidof(ID3D10BlendState));
_COM_SMARTPTR_TYPEDEF(ID3D10Buffer, __uuidof(ID3D10Buffer));
_COM_SMARTPTR_TYPEDEF(ID3D10Counter, __uuidof(ID3D10Counter));
_COM_SMARTPTR_TYPEDEF(ID3D10DepthStencilState, __uuidof(ID3D10DepthStencilState));
_COM_SMARTPTR_TYPEDEF(ID3D10DepthStencilView, __uuidof(ID3D10DepthStencilView));
_COM_SMARTPTR_TYPEDEF(ID3D10Device, __uuidof(ID3D10Device));
_COM_SMARTPTR_TYPEDEF(ID3D10DeviceChild, __uuidof(ID3D10DeviceChild));
_COM_SMARTPTR_TYPEDEF(ID3D10GeometryShader, __uuidof(ID3D10GeometryShader));
_COM_SMARTPTR_TYPEDEF(ID3D10InputLayout, __uuidof(ID3D10InputLayout));
_COM_SMARTPTR_TYPEDEF(ID3D10Multithread, __uuidof(ID3D10Multithread));
_COM_SMARTPTR_TYPEDEF(ID3D10PixelShader, __uuidof(ID3D10PixelShader));
_COM_SMARTPTR_TYPEDEF(ID3D10Predicate, __uuidof(ID3D10Predicate));
_COM_SMARTPTR_TYPEDEF(ID3D10Query, __uuidof(ID3D10Query));
_COM_SMARTPTR_TYPEDEF(ID3D10RasterizerState, __uuidof(ID3D10RasterizerState));
_COM_SMARTPTR_TYPEDEF(ID3D10RenderTargetView, __uuidof(ID3D10RenderTargetView));
_COM_SMARTPTR_TYPEDEF(ID3D10Resource, __uuidof(ID3D10Resource));
_COM_SMARTPTR_TYPEDEF(ID3D10SamplerState, __uuidof(ID3D10SamplerState));
_COM_SMARTPTR_TYPEDEF(ID3D10ShaderResourceView, __uuidof(ID3D10ShaderResourceView));
_COM_SMARTPTR_TYPEDEF(ID3D10Texture1D, __uuidof(ID3D10Texture1D));
_COM_SMARTPTR_TYPEDEF(ID3D10Texture2D, __uuidof(ID3D10Texture2D));
_COM_SMARTPTR_TYPEDEF(ID3D10Texture3D, __uuidof(ID3D10Texture3D));
_COM_SMARTPTR_TYPEDEF(ID3D10VertexShader, __uuidof(ID3D10VertexShader));
_COM_SMARTPTR_TYPEDEF(ID3D10View, __uuidof(ID3D10View));

#endif

#ifdef __d3d10_1_h__

_COM_SMARTPTR_TYPEDEF(ID3D10BlendState1, __uuidof(ID3D10BlendState1));
_COM_SMARTPTR_TYPEDEF(ID3D10Device1, __uuidof(ID3D10Device1));
_COM_SMARTPTR_TYPEDEF(ID3D10ShaderResourceView1, __uuidof(ID3D10ShaderResourceView1));

#endif

#ifdef __D3D10EFFECT_H__

_COM_SMARTPTR_TYPEDEF(ID3D10Effect, IID_ID3D10Effect);
_COM_SMARTPTR_TYPEDEF(ID3D10EffectBlendVariable, IID_ID3D10EffectBlendVariable);
_COM_SMARTPTR_TYPEDEF(ID3D10EffectConstantBuffer, IID_ID3D10EffectConstantBuffer);
_COM_SMARTPTR_TYPEDEF(ID3D10EffectDepthStencilVariable, IID_ID3D10EffectDepthStencilVariable);
_COM_SMARTPTR_TYPEDEF(ID3D10EffectDepthStencilViewVariable, IID_ID3D10EffectDepthStencilViewVariable);
_COM_SMARTPTR_TYPEDEF(ID3D10EffectMatrixVariable, IID_ID3D10EffectMatrixVariable);
_COM_SMARTPTR_TYPEDEF(ID3D10EffectPool, IID_ID3D10EffectPool);
_COM_SMARTPTR_TYPEDEF(ID3D10EffectRasterizerVariable, IID_ID3D10EffectRasterizerVariable);
_COM_SMARTPTR_TYPEDEF(ID3D10EffectRenderTargetViewVariable, IID_ID3D10EffectRenderTargetViewVariable);
_COM_SMARTPTR_TYPEDEF(ID3D10EffectSamplerVariable, IID_ID3D10EffectSamplerVariable);
_COM_SMARTPTR_TYPEDEF(ID3D10EffectScalarVariable, IID_ID3D10EffectScalarVariable);
_COM_SMARTPTR_TYPEDEF(ID3D10EffectShaderResourceVariable, IID_ID3D10EffectShaderResourceVariable);
_COM_SMARTPTR_TYPEDEF(ID3D10EffectShaderVariable, IID_ID3D10EffectShaderVariable);
_COM_SMARTPTR_TYPEDEF(ID3D10EffectStringVariable, IID_ID3D10EffectStringVariable);
_COM_SMARTPTR_TYPEDEF(ID3D10EffectVectorVariable, IID_ID3D10EffectVectorVariable);

#endif

#ifdef __dxgi_h__

_COM_SMARTPTR_TYPEDEF(IDXGIAdapter, __uuidof(IDXGIAdapter));
_COM_SMARTPTR_TYPEDEF(IDXGIAdapter1, __uuidof(IDXGIAdapter1));
_COM_SMARTPTR_TYPEDEF(IDXGIDevice, __uuidof(IDXGIDevice));
_COM_SMARTPTR_TYPEDEF(IDXGIDevice1, __uuidof(IDXGIDevice1));
_COM_SMARTPTR_TYPEDEF(IDXGIDeviceSubObject, __uuidof(IDXGIDeviceSubObject));
_COM_SMARTPTR_TYPEDEF(IDXGIFactory, __uuidof(IDXGIFactory));
_COM_SMARTPTR_TYPEDEF(IDXGIFactory1, __uuidof(IDXGIFactory1));
_COM_SMARTPTR_TYPEDEF(IDXGIKeyedMutex, __uuidof(IDXGIKeyedMutex));
_COM_SMARTPTR_TYPEDEF(IDXGIObject, __uuidof(IDXGIObject));
_COM_SMARTPTR_TYPEDEF(IDXGIOutput, __uuidof(IDXGIOutput));
_COM_SMARTPTR_TYPEDEF(IDXGIResource, __uuidof(IDXGIResource));
_COM_SMARTPTR_TYPEDEF(IDXGISurface, __uuidof(IDXGISurface));
_COM_SMARTPTR_TYPEDEF(IDXGISurface1, __uuidof(IDXGISurface1));
_COM_SMARTPTR_TYPEDEF(IDXGISwapChain, __uuidof(IDXGISwapChain));

#endif

#ifdef DWRITE_H_INCLUDED

_COM_SMARTPTR_TYPEDEF(IDWriteBitmapRenderTarget, __uuidof(IDWriteBitmapRenderTarget));
_COM_SMARTPTR_TYPEDEF(IDWriteFactory, __uuidof(IDWriteFactory));
_COM_SMARTPTR_TYPEDEF(IDWriteFont, __uuidof(IDWriteFont));
_COM_SMARTPTR_TYPEDEF(IDWriteFontCollection, __uuidof(IDWriteFontCollection));
_COM_SMARTPTR_TYPEDEF(IDWriteFontCollectionLoader, __uuidof(IDWriteFontCollectionLoader));
_COM_SMARTPTR_TYPEDEF(IDWriteFontFace, __uuidof(IDWriteFontFace));
_COM_SMARTPTR_TYPEDEF(IDWriteFontFamily, __uuidof(IDWriteFontFamily));
_COM_SMARTPTR_TYPEDEF(IDWriteFontFile, __uuidof(IDWriteFontFile));
_COM_SMARTPTR_TYPEDEF(IDWriteFontFileEnumerator, __uuidof(IDWriteFontFileEnumerator));
_COM_SMARTPTR_TYPEDEF(IDWriteFontFileLoader, __uuidof(IDWriteFontFileLoader));
_COM_SMARTPTR_TYPEDEF(IDWriteFontFileStream, __uuidof(IDWriteFontFileStream));
_COM_SMARTPTR_TYPEDEF(IDWriteFontList, __uuidof(IDWriteFontList));
_COM_SMARTPTR_TYPEDEF(IDWriteGdiInterop, __uuidof(IDWriteGdiInterop));
_COM_SMARTPTR_TYPEDEF(IDWriteGlyphRunAnalysis, __uuidof(IDWriteGlyphRunAnalysis));
_COM_SMARTPTR_TYPEDEF(IDWriteInlineObject, __uuidof(IDWriteInlineObject));
_COM_SMARTPTR_TYPEDEF(IDWriteLocalFontFileLoader, __uuidof(IDWriteLocalFontFileLoader));
_COM_SMARTPTR_TYPEDEF(IDWriteLocalizedStrings, __uuidof(IDWriteLocalizedStrings));
_COM_SMARTPTR_TYPEDEF(IDWriteNumberSubstitution, __uuidof(IDWriteNumberSubstitution));
_COM_SMARTPTR_TYPEDEF(IDWritePixelSnapping, __uuidof(IDWritePixelSnapping));
_COM_SMARTPTR_TYPEDEF(IDWriteRenderingParams, __uuidof(IDWriteRenderingParams));
_COM_SMARTPTR_TYPEDEF(IDWriteTextAnalysisSink, __uuidof(IDWriteTextAnalysisSink));
_COM_SMARTPTR_TYPEDEF(IDWriteTextAnalysisSource, __uuidof(IDWriteTextAnalysisSource));
_COM_SMARTPTR_TYPEDEF(IDWriteTextAnalyzer, __uuidof(IDWriteTextAnalyzer));
_COM_SMARTPTR_TYPEDEF(IDWriteTextFormat, __uuidof(IDWriteTextFormat));
_COM_SMARTPTR_TYPEDEF(IDWriteTextLayout, __uuidof(IDWriteTextLayout));
_COM_SMARTPTR_TYPEDEF(IDWriteTextRenderer, __uuidof(IDWriteTextRenderer));
_COM_SMARTPTR_TYPEDEF(IDWriteTypography, __uuidof(IDWriteTypography));

#endif

#ifdef __wincodec_h__

_COM_SMARTPTR_TYPEDEF(IWICBitmap, __uuidof(IWICBitmap));
_COM_SMARTPTR_TYPEDEF(IWICBitmapClipper, __uuidof(IWICBitmapClipper));
_COM_SMARTPTR_TYPEDEF(IWICBitmapCodecInfo, __uuidof(IWICBitmapCodecInfo));
_COM_SMARTPTR_TYPEDEF(IWICBitmapCodecProgressNotification, __uuidof(IWICBitmapCodecProgressNotification));
_COM_SMARTPTR_TYPEDEF(IWICBitmapDecoder, __uuidof(IWICBitmapDecoder));
_COM_SMARTPTR_TYPEDEF(IWICBitmapDecoderInfo, __uuidof(IWICBitmapDecoderInfo));
_COM_SMARTPTR_TYPEDEF(IWICBitmapEncoder, __uuidof(IWICBitmapEncoder));
_COM_SMARTPTR_TYPEDEF(IWICBitmapEncoderInfo, __uuidof(IWICBitmapEncoderInfo));
_COM_SMARTPTR_TYPEDEF(IWICBitmapFlipRotator, __uuidof(IWICBitmapFlipRotator));
_COM_SMARTPTR_TYPEDEF(IWICBitmapFrameDecode, __uuidof(IWICBitmapFrameDecode));
_COM_SMARTPTR_TYPEDEF(IWICBitmapFrameEncode, __uuidof(IWICBitmapFrameEncode));
_COM_SMARTPTR_TYPEDEF(IWICBitmapLock, __uuidof(IWICBitmapLock));
_COM_SMARTPTR_TYPEDEF(IWICBitmapScaler, __uuidof(IWICBitmapScaler));
_COM_SMARTPTR_TYPEDEF(IWICBitmapSource, __uuidof(IWICBitmapSource));
_COM_SMARTPTR_TYPEDEF(IWICBitmapSourceTransform, __uuidof(IWICBitmapSourceTransform));
_COM_SMARTPTR_TYPEDEF(IWICColorContext, __uuidof(IWICColorContext));
_COM_SMARTPTR_TYPEDEF(IWICColorTransform, __uuidof(IWICColorTransform));
_COM_SMARTPTR_TYPEDEF(IWICComponentInfo, __uuidof(IWICComponentInfo));
_COM_SMARTPTR_TYPEDEF(IWICDevelopRaw, __uuidof(IWICDevelopRaw));
_COM_SMARTPTR_TYPEDEF(IWICDevelopRawNotificationCallback, __uuidof(IWICDevelopRawNotificationCallback));
_COM_SMARTPTR_TYPEDEF(IWICEnumMetadataItem, __uuidof(IWICEnumMetadataItem));
_COM_SMARTPTR_TYPEDEF(IWICFastMetadataEncoder, __uuidof(IWICFastMetadataEncoder));
_COM_SMARTPTR_TYPEDEF(IWICFormatConverter, __uuidof(IWICFormatConverter));
_COM_SMARTPTR_TYPEDEF(IWICFormatConverterInfo, __uuidof(IWICFormatConverterInfo));
_COM_SMARTPTR_TYPEDEF(IWICImagingFactory, __uuidof(IWICImagingFactory));
_COM_SMARTPTR_TYPEDEF(IWICMetadataBlockReader, __uuidof(IWICMetadataBlockReader));
_COM_SMARTPTR_TYPEDEF(IWICMetadataBlockWriter, __uuidof(IWICMetadataBlockWriter));
_COM_SMARTPTR_TYPEDEF(IWICMetadataQueryReader, __uuidof(IWICMetadataQueryReader));
_COM_SMARTPTR_TYPEDEF(IWICMetadataQueryWriter, __uuidof(IWICMetadataQueryWriter));
_COM_SMARTPTR_TYPEDEF(IWICPalette, __uuidof(IWICPalette));
_COM_SMARTPTR_TYPEDEF(IWICPixelFormatInfo, __uuidof(IWICPixelFormatInfo));
_COM_SMARTPTR_TYPEDEF(IWICPixelFormatInfo2, __uuidof(IWICPixelFormatInfo2));
_COM_SMARTPTR_TYPEDEF(IWICProgressCallback, __uuidof(IWICProgressCallback));
_COM_SMARTPTR_TYPEDEF(IWICProgressiveLevelControl, __uuidof(IWICProgressiveLevelControl));
_COM_SMARTPTR_TYPEDEF(IWICStream, __uuidof(IWICStream));

#endif