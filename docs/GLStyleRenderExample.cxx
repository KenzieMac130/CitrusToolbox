/* External setup */
...

/* Setup Viewport/Matrix */
glViewport(camera.x,
		   camera.y,
		   camera.w,
		   camera.h);

glScissor(camera.x,
		   camera.y,
		   camera.w,
		   camera.h);

/* Setup Camera */
glBindBufferBase(GL_UNIFORM_BUFFER, 0, camera.ubo);

for(auto const& renderable : visibleRenderables)
{	
	const auto& mesh = renderable.mesh;
	
	/* Setup transforms */
	glBindBufferBase(GL_UNIFORM_BUFFER, 1, renderable.tranform);
	
	/* Bind Vertex/Index Buffer */
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, mesh.indexBuffer);
	glBindBuffers(GL_ARRAY_BUFFER, 0,
				  mesh.vertexBuffers.Count(),
				  mesh.vertexBuffers.Data());

	for(auto const& submesh : mesh.submeshes) {
		const auto& material = submesh.material;
		
		/* Bind program */
		glUseProgram(material.shader.program[passIdx]);
		
		/* Set state */
		if(material.shader.depthTest) {
			glEnable(GL_DEPTH_TEST);
		} else {
			glDisable(GL_DEPTH_TEST);
		}
		if(material.shader.blend) {
			glEnable(GL_BLEND);
		} else {
			glDisable(GL_BLEND);
		}
		/* This can go on for a while... */
		...
		
		/* Bind Material Buffer */
		glBindBufferBase(GL_UNIFORM_BUFFER, 2, material.ubo);
		
		/* Bind Textures */
		glBindTextures(0, material.textures.count(), material.textures.data());
			
		/* Draw */
		glDrawArrays(GL_TRIANGLES, submesh.beginIdx, submesh.idxCount);
	}
}




