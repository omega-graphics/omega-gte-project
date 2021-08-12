#include "omegaGTE/GTEShaderTypes.h"

#include "omegaGTE/GTEBase.h"
#include "omegaGTE/TE.h"


_NAMESPACE_BEGIN_

SharedHandle<OmegaTessalationEngineContext> CreateNativeRenderTargetTEContext(SharedHandle<GENativeRenderTarget> & renderTarget){

};

SharedHandle<OmegaTessalationEngineContext> CreateTextureRenderTargetTEContext(SharedHandle<GETextureRenderTarget> & renderTarget){

};

OmegaGTETexturedVertex *convertVertex(OmegaGTE::GETexturedVertex & vertex){
    auto v = new OmegaGTETexturedVertex();
    v->pos = {vertex.pos.getI(),vertex.pos.getJ(),vertex.pos.getK()};
    v->texturePos = {vertex.textureCoord.getI(),vertex.textureCoord.getJ()};
    return v;
};

OmegaGTEColorVertex *convertVertex(OmegaGTE::GEColoredVertex & vertex){
    auto v = new OmegaGTEColorVertex();
    v->pos = {vertex.pos.getI(),vertex.pos.getJ(),vertex.pos.getK()};
    v->color = {vertex.color.valueAt(1,1),vertex.color.valueAt(1,2),vertex.color.valueAt(1,3),vertex.color.valueAt(1,4)};
    return v;
};

_NAMESPACE_END_
