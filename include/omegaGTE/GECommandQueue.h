#include "GTEBase.h"
#include "GEPipeline.h"
#include "GERenderTarget.h"
#include "GETexture.h"
#include <vector>

#ifndef OMEGAGTE_GECOMMANDQUEUE_H
#define OMEGAGTE_GECOMMANDQUEUE_H

_NAMESPACE_BEGIN_
    class GEBuffer;
    class GEFence;

    struct GEScissorRect;
    struct GEViewport;

    /// @brief Describes a Render Pass
    struct  OMEGAGTE_EXPORT GERenderPassDescriptor {
        GENativeRenderTarget *nRenderTarget = nullptr;
        GETextureRenderTarget *tRenderTarget = nullptr;
        typedef GERenderTarget::RenderPassDesc::ColorAttachment ColorAttachment;
        ColorAttachment *colorAttachment;
        bool multisampleResolve = false;
        typedef GERenderTarget::RenderPassDesc::MultisampleResolveDesc MultisampleResolveDesc;
        MultisampleResolveDesc resolveDesc;
    };

    /// @brief Describes a Compute Pass
    struct  OMEGAGTE_EXPORT GEComputePassDescriptor {

    };

    /**
     A Reusable interface for directly uploading commands to a GPU.
     */
    class  OMEGAGTE_EXPORT GECommandBuffer : public GTEResource {
        friend class GERenderTarget::CommandBuffer;
    protected:
        typedef GERenderTarget::CommandBuffer::PolygonType RenderPassDrawPolygonType;
    private:
         /**
         Render Pass
         */
        virtual void startRenderPass(const GERenderPassDescriptor & desc) = 0;
        virtual void setRenderPipelineState(SharedHandle<GERenderPipelineState> & pipelineState) = 0;
        //
        virtual void setVertexBuffer(SharedHandle<GEBuffer> & buffer) = 0;
        
        virtual void bindResourceAtVertexShader(SharedHandle<GEBuffer> & buffer,unsigned id) = 0;
        virtual void bindResourceAtVertexShader(SharedHandle<GETexture> & texture,unsigned id) = 0;
        
        virtual void bindResourceAtFragmentShader(SharedHandle<GEBuffer> & buffer,unsigned id) = 0;
        virtual void bindResourceAtFragmentShader(SharedHandle<GETexture> & texture,unsigned id) = 0;

        virtual void setStencilRef(unsigned ref) = 0;
        
        virtual void setViewports(std::vector<GEViewport> viewport) = 0;
        virtual void setScissorRects(std::vector<GEScissorRect> scissorRects) = 0;
        
        virtual void drawPolygons(RenderPassDrawPolygonType polygonType,unsigned vertexCount,size_t startIdx) = 0;
        virtual void finishRenderPass() = 0;
        /**
         Compute Pass
        */
    public:

        /**
        Blit Pass
        */
        virtual void startBlitPass() = 0;
        virtual void copyTextureToTexture(SharedHandle<GETexture> & src,SharedHandle<GETexture> & dest) = 0;
        virtual void copyTextureToTexture(SharedHandle<GETexture> & src,SharedHandle<GETexture> & dest,const TextureRegion & region,const GPoint3D & destCoord) = 0;
        virtual void finishBlitPass() = 0;

        /// @brief Starts a Compute Pass.
        /// @param desc A GEComputePassDescriptor describing the Compute Pass.
        /// @paragraph This method must be called before dispatch commands can be encoded.
        virtual void startComputePass(const GEComputePassDescriptor & desc) = 0;

        /// @brief Sets a Compute Pipeline State in a Compute Pass.
        /// @param pipelineState A GEComputePipelineState
        /// @paragraph This method can be called after startComputePass() has been called and must be invoked before a dispatch command is encoded.
        virtual void setComputePipelineState(SharedHandle<GEComputePipelineState> & pipelineState) = 0;

        /// @brief Binds a Buffer Resource to a Descriptor in the scope of the Compute Shader.
        /// @param buffer The Resource to bind.
        /// @param id The OmegaSL Binding id.
        virtual void bindResourceAtComputeShader(SharedHandle<GEBuffer> & buffer,unsigned id) = 0;

        /// @brief Binds a Texture Resource to a Descriptor in the scope of the Compute Shader.
        /// @param texture The Resource to bind.
        /// @param id The OmegaSL Binding id.
        virtual void bindResourceAtComputeShader(SharedHandle<GETexture> & texture,unsigned id) = 0;

        /// @brief Executes a Compute Pipeline (Encodes a dispatch command in the Compute Pass).
        /// @param x The Number of ThreadGroups dispatched in the `x` direction.
        /// @param y The Number of ThreadGroups dispatched in the `y` direction.
        /// @param z The Number of ThreadGroups dispatched in the `z` direction.
        virtual void dispatchThreads(unsigned x,unsigned y,unsigned z) = 0;

        /// @brief Finish encoding a Compute Pass.
        /// @paragraph This method must be invoked when a dispatch command has been encoded.
        virtual void finishComputePass() = 0;

        virtual void reset() = 0;
        virtual ~GECommandBuffer(){};
    };

    class  OMEGAGTE_EXPORT GECommandQueue : public GTEResource {
        unsigned size;
    protected:
        unsigned currentlyOccupied = 0;
        GECommandQueue(unsigned size);
    public:
        virtual SharedHandle<GECommandBuffer> getAvailableBuffer() = 0;
        unsigned getSize();

        /// @brief Encodes a wait on command buffer using fence.
        /// @param commandBuffer The GECommandBuffer to encode the wait on.
        /// @param waitFence The GEFence to wait for.
        /// @paragraph Allows sync between command buffers in different queues.
        virtual void notifyCommandBuffer(SharedHandle<GECommandBuffer> & commandBuffer,SharedHandle<GEFence> & waitFence) = 0;

        /// @brief Submits command buffer to Queue.
        /// @param commandBuffer The GECommandBuffer to submit
        /// @paragraph Does not sync between commandBuffers
        virtual void submitCommandBuffer(SharedHandle<GECommandBuffer> & commandBuffer) = 0;

        /// @brief Submits command buffer to Queue and encodes a signal event on completion.
        /// @param commandBuffer The GECommandBuffer to submit
        /// @param signalFence The GEFence to signal.
        /// @paragraph Allows sync between command buffers in different queues.
        virtual void submitCommandBuffer(SharedHandle<GECommandBuffer> & commandBuffer,SharedHandle<GEFence> & signalFence) = 0;
        virtual void commitToGPU() = 0;
        virtual void commitToGPUAndWait() = 0;
        virtual ~GECommandQueue(){};
    };
_NAMESPACE_END_

#endif
