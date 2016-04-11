/*
 Copyright 2016 Google Inc.
 
 Licensed under the Apache License, Version 2.0 (the "License");
 you may not use this file except in compliance with the License.
 You may obtain a copy of the License at
 
 http://www.apache.org/licenses/LICENSE-2.0
 
 Unless required by applicable law or agreed to in writing, software
 distributed under the License is distributed on an "AS IS" BASIS,
 WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 See the License for the specific language governing permissions and
 limitations under the License.


 Copyright (c) 2016, The Cinder Project, All rights reserved.

 This code is intended for use with the Cinder C++ library: http://libcinder.org

 Redistribution and use in source and binary forms, with or without modification, are permitted provided that
 the following conditions are met:

    * Redistributions of source code must retain the above copyright notice, this list of conditions and
	the following disclaimer.
    * Redistributions in binary form must reproduce the above copyright notice, this list of conditions and
	the following disclaimer in the documentation and/or other materials provided with the distribution.

 THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED
 WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A
 PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR
 ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED
 TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
 HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING
 NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
 POSSIBILITY OF SUCH DAMAGE.
*/

#pragma once

#include "cinder/vk/BaseVkObject.h"
#include "cinder/vk/UniformLayout.h"
#include "cinder/vk/VertexBufferMesh.h"
#include "cinder/DataSource.h"
#include "cinder/GeomIo.h"

namespace cinder { namespace vk {

class ShaderProg;
using ShaderProgRef = std::shared_ptr<ShaderProg>;


//! \class ShaderDef
//!
//!
class ShaderDef {
public:
	ShaderDef();

	ShaderDef&				color();	
	ShaderDef&				texture();
	ShaderDef&				lambert();
	// Used by draw(TextureRef&) stock shader; scales ciPosition and ciTexCoord according to
	// uniform "uPositionScale", "uPositionOffset", "uTexCoord0Scale", "uTexCoord0Offset"
	ShaderDef&				uniformBasedPosAndTexCoord();

	bool operator<( const ShaderDef &rhs ) const;
	
protected:
	bool					mColor;
	bool					mTextureMapping;
	bool					mTextureUnormalizedCoordinates;
	bool					mLambert;
	bool					mUniformBasedPosAndTexCoord;
	
	friend class Context;
};

//! \class ShaderProg
//!
//!
class ShaderProg : public BaseDeviceObject {
public:

	// Argument can be blockName or blockName.uniformName format
	using BlockNameTranslateFn = std::function<std::string(const std::string&)>;

	//! \class Attribute
	//!
	struct Attribute {

		Attribute( geom::Attrib semantic, int32_t location, int32_t binding, GlslAttributeDataType type ) 
			: mSemantic( semantic ), mLocation( location ), mBinding( binding ), mType( type ) {}
		Attribute( const std::string& name, geom::Attrib semantic, int32_t location, int32_t binding, GlslAttributeDataType type ) 
			: mName( name ), mSemantic( semantic ), mLocation( location ), mBinding( binding ), mType( type ) {}
		virtual ~Attribute() {}

		//! Returns a const reference of the name as defined in the Vertex Shader.
		const std::string&		getName() const { return mName; }
		//! Returns the number of attributes expected by the Vertex Shader. mCount will be 1 unless this attribute is an array.
		int32_t					getCount() const { return mCount; }
		//! Returns the Vertex Shader generated or user defined location of this attribute.
		int32_t					getLocation() const { return mLocation; }
		//! Returns the GLenum representation of the type of this attribute (for example, \c GL_FLOAT_VEC3)
		int32_t					getBinding() const { return mBinding; }
		//! Returns the Vertex Shader generated or user defined location of this attribute.
		GlslAttributeDataType	getType() const { return mType; }
		//! Returns the defined geom::Attrib semantic.
		geom::Attrib			getSemantic() const { return mSemantic; }

	  private:
		std::string				mName;
		int32_t					mCount = 0;
		int32_t					mLocation = -1;
		int32_t					mBinding = -1;
		GlslAttributeDataType	mType = glsl_attr_unknown;
		geom::Attrib			mSemantic = geom::Attrib::USER_DEFINED;

		friend class ShaderProg;
	};

	//! \class Format
	//!
	//!
	class Format {
	public:

		class ShaderData {
		public:
			enum class Type { UNKNOWN, GLSL, SPIRV };

			ShaderData() {}
			virtual ~ShaderData() {}

			bool							isInitialized() const { return mInitialized; }
			ShaderData::Type				getDataType() const { return mDataType; }
			const std::string&				getSourceText() const { return mSourceText; }
			const std::vector<uint32_t>&	getSpirvBinary() const { return mSpirvBinary; }
			const std::string&				getEntryPoint() const { return mEntryPoint; }

		private:
			bool					mInitialized = false;
			ShaderData::Type		mDataType = ShaderData::Type::UNKNOWN;
			DataSourceRef			mDataSource;
			std::string				mSourceText;
			std::vector<uint32_t>	mSpirvBinary;
			std::string				mEntryPoint = "main";

			void					setShaderData( const DataSourceRef &dataSource, const std::string& entryPoint );
			void					setShaderData( const std::string &sourceText, const std::string& entryPoint );
			void					setShaderData( const std::vector<uint32_t> &spirvBinary, const std::string& entryPoint );
			void					setEntryPoint( const std::string& entryPoint );
			void					loadShader();

			friend class ShaderProg::Format;
			friend class ShaderProg;
		};

		// -----------------------------------------------------------------------------------------

		Format();
		virtual ~Format() {}

		Format&			vertex( const DataSourceRef &dataSource, const std::string& entryPoint = "" );
		Format&			vertex( const std::string &souceText, const std::string& entryPoint = "" );
		Format&			vertex( const std::vector<uint32_t> &spirvBinary, const std::string& entryPoint = "" );
		Format&			fragment( const DataSourceRef &dataSource, const std::string& entryPoint = "" );
		Format&			fragment( const std::string &souceText, const std::string& entryPoint = "" );
		Format&			fragment( const std::vector<uint32_t> &spirvBinary, const std::string& entryPoint = "" );
		Format&			geometry( const DataSourceRef &dataSource, const std::string& entryPoint = "" );
		Format&			geometry( const std::string &souceText, const std::string& entryPoint = "" );
		Format&			geometry( const std::vector<uint32_t> &spirvBinary, const std::string& entryPoint = "" );
		Format&			tessellationCtrl( const DataSourceRef &dataSource, const std::string& entryPoint = "" );
		Format&			tessellationCtrl( const std::string &souceText, const std::string& entryPoint = "" );
		Format&			tessellationCtrl( const std::vector<uint32_t> &spirvBinary, const std::string& entryPoint = "" );
		Format&			tessellationEval( const DataSourceRef &dataSource, const std::string& entryPoint = "" );
		Format&			tessellationEval( const std::string &souceText, const std::string& entryPoint = "" );
		Format&			tessellationEval( const std::vector<uint32_t> &spirvBinary, const std::string& entryPoint = "" );
		Format&			compute( const DataSourceRef &dataSource, const std::string& entryPoint = "" );
		Format&			compute( const std::string &souceText, const std::string& entryPoint = "" );
		Format&			compute( const std::vector<uint32_t> &spirvBinary, const std::string& entryPoint = "" );

/*
		const Format::ShaderData&	getVertexShaderData() const;
		const Format::ShaderData&	getFragmentShaderData() const;
		const Format::ShaderData&	getGeometryShaderData() const;
		const Format::ShaderData&	getTessellationCtrlShaderData() const;
		const Format::ShaderData&	getTessellationEvalShaderData() const;
		const Format::ShaderData&	getComputeShaderData() const;


		const std::string&				getVertexSource() const { return mVertexSource; }
		const std::string&				getFragmentSource() const { return mFragmentSource; }
		const std::string&				getGeometrySource() const { return mGeometrySource; }
		const std::string&				getTessellationCtrlSource() const { return mTessellationCtrlSource; }
		const std::string&				getTessellationEvalSource() const { return mTessellationEvalSource; }
		const std::string&				getComputeSource() const { return mComputeSource; }

		const std::vector<uint32_t>&	getVertexSpirv() const { return mVertexSpirv; }
		const std::vector<uint32_t>&	getFragmentSpirv() const { return mFragmentSpirv; }
		const std::vector<uint32_t>&	getGeometrySpirv() const { return mGeometrySpirv; }
		const std::vector<uint32_t>&	getTessellationCtrlSpirv() const { return mTessellationCtrlSpirv; }
		const std::vector<uint32_t>&	getTessellationEvalSpirv() const { return mTessellationEvalSpirv; }
		const std::vector<uint32_t>&	getComputeSpirv() const { return mComputeSpirv; }

		bool			hasVertex()           const { return ( ( ! mVertexSpirv.empty() )           || ( ! mVertexSource.empty()          ) ); }
		bool			hasFragment()         const { return ( ( ! mFragmentSpirv.empty() )         || ( ! mFragmentSpirv.empty()         ) ); }
		bool			hasGeometry()         const { return ( ( ! mGeometrySpirv.empty() )         || ( ! mGeometrySpirv.empty()         ) ); }
		bool			hasTessellationCtrl() const { return ( ( ! mTessellationCtrlSpirv.empty() ) || ( ! mTessellationCtrlSpirv.empty() ) ); }
		bool			hasTessellationEval() const { return ( ( ! mTessellationEvalSpirv.empty() ) || ( ! mTessellationEvalSpirv.empty() ) ); }
		bool			hasCompute()          const { return ( ( ! mComputeSpirv.empty() )          || ( ! mComputeSpirv.empty()          ) ); }
*/

		Format&			binding( const std::string& bindingName, uint32_t bindingNumber, uint32_t setNumber = DEFAULT_SET );
		Format&			set( uint32_t setNumber, uint32_t changeFrequency = CHANGES_DONTCARE );
		Format&			attribute( geom::Attrib semantic, int32_t location, int32_t binding, GlslAttributeDataType type );
		Format&			uniformLayout( const UniformLayout& layout );
		bool			userDefinedUniformLayout() const { return mUserDefinedUniformLayout; }

		Format&			blockNameTranslateFn( BlockNameTranslateFn fn );

	private:

		std::map<VkShaderStageFlagBits, Format::ShaderData>	mShaderData;

		struct Binding {
			uint32_t bindingNumber	= INVALID_BINDING;
			uint32_t setNumber		= DEFAULT_SET;
			Binding() {}
			Binding( uint32_t aBindingNumber, uint32_t aSetNumber ) : bindingNumber( aBindingNumber ), setNumber( aSetNumber ) {}
			virtual ~Binding() {}
		};

		std::map<std::string, Binding>	mBindings;		// bindingName = binding
		std::map<uint32_t, uint32_t>	mSets;			// setNumber = changeFrequency
		std::vector<Attribute>			mAttributes;
		UniformLayout					mUniformLayout;
		bool							mUserDefinedUniformLayout = false;

		BlockNameTranslateFn			mBlockNameTranslateFn;

		///const Format::ShaderDataRef& getAllocatedShaderData( VkShaderStageFlagBits shaderStage );	
		//void setShaderData( const DataSourceRef &dataSource, std::string *shaderSourceDest, fs::path *shaderPathDest, std::vector<uint32_t> *shaderSpirvDest, ShaderDataType *shaderDataType );

		friend class ShaderProg;
	};

	ShaderProg( const ShaderProg::Format &format, vk::Device *device );
	virtual ~ShaderProg();

	static ShaderProgRef	create( const ShaderProg::Format &format, vk::Device *device = nullptr );
	static ShaderProgRef	create( DataSourceRef vertexShader, DataSourceRef fragmentShader , vk::Device *device = nullptr );
	static ShaderProgRef	create( const std::string &vertexShader, const std::string &fragmentShader, vk::Device *device = nullptr );

	bool								isCompute() const;

	const std::vector<VkPipelineShaderStageCreateInfo>&		getPipelineShaderStages() const { return mPipelineShaderStages; }

	const std::vector<Attribute>&		getActiveAttributes() const { return mAttributes; }
	const vk::VertexBufferMesh::Layout&	getVertexLayout() const { return mVertexLayout; }
	const UniformLayout&				getUniformLayout() const { return mUniformLayout; }

	void								uniform( const std::string& name, const float    value );
	void								uniform( const std::string& name, const int32_t  value );
	void								uniform( const std::string& name, const uint32_t value );
	void								uniform( const std::string& name, const bool     value );
	void								uniform( const std::string& name, const vec2&    value );
	void								uniform( const std::string& name, const vec3&    value );
	void								uniform( const std::string& name, const vec4&    value );
	void								uniform( const std::string& name, const ivec2&   value );
	void								uniform( const std::string& name, const ivec3&   value );
	void								uniform( const std::string& name, const ivec4&   value );
	void								uniform( const std::string& name, const uvec2&   value );
	void								uniform( const std::string& name, const uvec3&   value );
	void								uniform( const std::string& name, const uvec4&   value );
	void								uniform( const std::string& name, const mat2&    value );
	void								uniform( const std::string& name, const mat3&    value );
	void								uniform( const std::string& name, const mat4&    value );
	void								uniform( const std::string& name, const TextureBaseRef& texture );

	static std::string					defaultBlockNameTranslate( const std::string& name );

private:
	std::vector<std::string>						mEntryPoints;
	std::vector<VkPipelineShaderStageCreateInfo>	mPipelineShaderStages;
	
	std::vector<Attribute>							mAttributes;
	vk::VertexBufferMesh::Layout					mVertexLayout;
	UniformLayout									mUniformLayout;

	void initialize( const ShaderProg::Format &format );
	void destroy( bool removeFromTracking = true );
	friend class vk::Device;
};

// -------------------------------------------------------------------------------------------------
// Mapped type names
// -------------------------------------------------------------------------------------------------
using GlslProg = ShaderProg;
using GlslProgRef = ShaderProgRef;

}} // namespace cinder::vk