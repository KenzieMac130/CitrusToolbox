/* External setup */
...

/* Setup Viewport/Matrix */
vkCmdSetViewport(cmd,
			0, 1,
			(VkRect2D){
				camera.x,
				camera.y,
				camera.w,
				camera.h});

vkCmdSetScissor(cmd,
			0, 1,
			(VkRect2D){
				camera.x,
				camera.y,
				camera.w,
				camera.h});

for(auto const& renderable : visibleRenderables)
{	
	const auto& mesh = renderable.mesh;
	
	/* Bind Vertex/Index Buffer */
	vkCmdBindIndexBuffer(cmd, mesh.indexBuffer, 0, VK_INDEX_TYPE_UINT32);
	vkCmdBindVertexBuffers(cmd, 0
				  mesh.vertexBuffers.Count(),
				  mesh.vertexBuffers.Data(),
				  mesh.vertexOffsets.Data());

	for(auto const& submesh : mesh.submeshes) {
		const auto& material = submesh.material;
		
		/* Bind pipeline */
		vkCmdBindPipeline(cmd, 
		VK_PIPELINE_BIND_POINT_GRAPHICS,
		material.shader.pipeline[passIdx]);
		
		/* Bind Descriptor Sets */
		VkDescriptorSet descSets[] = {
			camera.descSet,
			renderable.descSet,
			material.descSet
		}
		vkCmdBindDescriptorSets(cmd,
				VK_PIPELINE_BIND_POINT_GRAPHICS,
				material.shader.pipelineLayout[passIdx],
				0,
				3,
				descSets,
				0, NULL);
			
		/* Draw */
		vkCmdDrawIndexed(cmd, submesh.idxCount, 1, submesh.beginIdx, 0, 0);
	}
}




