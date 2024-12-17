/*
 * Copyright (c) 2024
 *	Side Effects Software Inc.  All rights reserved.
 *
 * Redistribution and use of Houdini Development Kit samples in source and
 * binary forms, with or without modification, are permitted provided that the
 * following conditions are met:
 * 1. Redistributions of source code must retain the above copyright notice,
 *    this list of conditions and the following disclaimer.
 * 2. The name of Side Effects Software may not be used to endorse or
 *    promote products derived from this software without specific prior
 *    written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY SIDE EFFECTS SOFTWARE `AS IS' AND ANY EXPRESS
 * OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES
 * OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED.  IN
 * NO EVENT SHALL SIDE EFFECTS SOFTWARE BE LIABLE FOR ANY DIRECT, INDIRECT,
 * INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT
 * LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA,
 * OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF
 * LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING
 * NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE,
 * EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 *
 *----------------------------------------------------------------------------
 * The Pmx SOP
 */

#include "SOP_Pmx.h"
#include "SOP_common.h"

// This is an automatically generated header file based on theDsFile, below,
// to provide SOP_PmxParms, an easy way to access parameter values from
// SOP_PmxVerb::cook with the correct type.
#include "SOP_Pmx.proto.h"

#include <GU/GU_Detail.h>
#include <OP/OP_Operator.h>
#include <OP/OP_OperatorTable.h>
#include <PRM/PRM_Include.h>
#include <PRM/PRM_TemplateBuilder.h>
#include <UT/UT_DSOVersion.h>
#include <UT/UT_Interrupt.h>
#include <UT/UT_StringHolder.h>
#include <SOP/SOP_Node.h>
#include <GU/GU_Detail.h>
#include <GA/GA_Attribute.h>
#include <GA/GA_AIFSharedStringTuple.h>
#include <UT/UT_StringArray.h>
#include <SYS/SYS_Math.h>
#include <SYS/SYS_Types.h>
#include <limits.h>
#include <iostream>
#include <typeinfo>
#include <algorithm> 
#include <stdint.h>
#include <windows.h>
#include <fmt/core.h>

using namespace UT::Literal;
using namespace HDK_Sample;

//
// Help is stored in a "wiki" style text file.  This text file should be copied
// to $HOUDINI_PATH/help/nodes/sop/star.txt
//
// See the sample_install.sh file for an example.
//

/// This is the internal name of the SOP type.
/// It isn't allowed to be the same as any other SOP's type name.
const UT_StringHolder SOP_Pmx::theSOPTypeName("hdk_pmx"_sh);

/// newSopOperator is the hook that Houdini grabs from this dll
/// and invokes to register the SOP.  In this case, we add ourselves
/// to the specified operator table.
void
newSopOperator(OP_OperatorTable *table)
{
    table->addOperator(new OP_Operator(
        SOP_Pmx::theSOPTypeName,   // Internal name
        "Pmx",                     // UI name
        SOP_Pmx::myConstructor,    // How to build the SOP
        SOP_Pmx::buildTemplates(), // My parameters
        0,                          // Min # of sources
        0,                          // Max # of sources
        nullptr,                    // Custom local variables (none)
        OP_FLAG_GENERATOR));        // Flag it as generator
}

/// This is a multi-line raw string specifying the parameter interface
/// for this SOP.
static const char *theDsFile = R"THEDSFILE(
{
    name        parameters
    parm {
        name    "pmx_path"
        label   "PMX Path"
        type    file
        default { "D:/sparkle.pmx" }
        parmtag { "filechooser_pattern" "*.pmx" }
        parmtag { "filechooser_mode" "read" }
    }
    parm {
        name    "mode"
        label   "Mode"
        type    ordinal
        default { "1" }
        menu    {
            "skin"    "Skin"
            "bone"    "Bone"
        }
    }
    parm {
        name    "scale"
        label   "Scale"
        type    float
        default { "0.08" }
    }
}
)THEDSFILE";

PRM_Template*
SOP_Pmx::buildTemplates()
{
    static PRM_TemplateBuilder templ("SOP_Pmx.C"_sh, theDsFile);
    return templ.templates();
}

class SOP_PmxVerb : public SOP_NodeVerb
{
public:
    SOP_PmxVerb() {}
    virtual ~SOP_PmxVerb() {}

    virtual SOP_NodeParms *allocParms() const { return new SOP_PmxParms(); }
    virtual UT_StringHolder name() const { return SOP_Pmx::theSOPTypeName; }

    virtual CookMode cookMode(const SOP_NodeParms *parms) const { return COOK_GENERIC; }

    virtual void cook(const CookParms &cookparms) const;
    
    /// This static data member automatically registers
    /// this verb class at library load time.
    static const SOP_NodeVerb::Register<SOP_PmxVerb> theVerb;
};

// The static member variable definition has to be outside the class definition.
// The declaration is inside the class.
const SOP_NodeVerb::Register<SOP_PmxVerb> SOP_PmxVerb::theVerb;

const SOP_NodeVerb *
SOP_Pmx::cookVerb() const 
{ 
    return SOP_PmxVerb::theVerb.get();
}

struct BoneFlags {
    uint16_t flag;
    BoneFlags(uint16_t _flag) {
        flag = _flag;
    }
    bool contains(uint16_t _flag) {
        return flag & _flag;
    }
    inline static uint16_t INDEXED_TAIL_BONE    = 0b0000000000000001;
    inline static uint16_t ROTATABLE            = 0b0000000000000010;
    inline static uint16_t TRANSLATABLE         = 0b0000000000000100;
    inline static uint16_t VISIBLE              = 0b0000000000001000;
    inline static uint16_t ENABLED              = 0b0000000000010000;
    inline static uint16_t IK                   = 0b0000000000100000;
    inline static uint16_t INHERIT_ROTATION     = 0b0000000100000000;
    inline static uint16_t INHERIT_TRANSLATION  = 0b0000001000000000;
    inline static uint16_t FIXED_AXIS           = 0b0000010000000000;
    inline static uint16_t LOCAL_AXIS           = 0b0000100000000000;
    inline static uint16_t PHYSICS_AFTER_DEFORM = 0b0001000000000000;
    inline static uint16_t EXTERNAL_PARENT      = 0b0010000000000000;
};

static void read_skin(
    BinaryReader &br
    , GU_Detail *detail
    , float scale
    , uint8_t appendix_uv
    , uint8_t vertex_index_size
    , uint8_t texture_index_size
    , uint8_t material_index_size
    , uint8_t bone_index_size
    , uint8_t morph_index_size
    , uint8_t rigidbody_index_size
) {
    
    GA_Offset start_ptoff{};
    {
        auto npoints = br.read_LE<uint32_t>();
        start_ptoff = detail->appendPointBlock(npoints);
        auto edge_scale_attrs = GA_RWHandleF(detail->addFloatTuple(GA_ATTRIB_POINT, "edge_scale", 1, GA_Defaults(1.0)));
        auto bones_index = GA_RWHandleV4(detail->addFloatTuple(GA_ATTRIB_POINT, "bones_index", 4, GA_Defaults(-1.0)));
        auto bones_weight = GA_RWHandleV4(detail->addFloatTuple(GA_ATTRIB_POINT, "bones_weight", 4, GA_Defaults(-1.0)));
        auto uv_attrib = GA_RWHandleV3(detail->addTextureAttribute(GA_ATTRIB_POINT));

        for (auto i = 0; i < npoints; i++) {
            auto pos = br.read_float3();
            detail->setPos3(start_ptoff + i, UT_Vector3(pos[0], pos[1], -pos[2]) * scale);
            auto nrm = br.read_float3();
            auto uv = br.read_float2();
            uv_attrib.set(i, UT_Vector3(uv[0], 1.0 - uv[1], 0.0));
            for (auto j = 0; j < appendix_uv; j++) {
                br.read_float4();
            }
            auto weight_type = br.read_LE<uint8_t>();
            if (weight_type == 0) {
                float a = br.read_pmx_int(bone_index_size);
                bones_index.set(i, {a, -1, -1, -1});
                bones_weight.set(i, {1, -1, -1, -1});
            }
            else if (weight_type == 1) {
                float a = br.read_pmx_int(bone_index_size);
                float b = br.read_pmx_int(bone_index_size);
                auto w = br.read_LE<float>();
                bones_index.set(i, {a, b, -1, -1});
                bones_weight.set(i, {w, 1-w, -1, -1});
            }
            else if (weight_type == 2 || weight_type == 4) {
                float a = br.read_pmx_int(bone_index_size);
                float b = br.read_pmx_int(bone_index_size);
                float c = br.read_pmx_int(bone_index_size);
                float d = br.read_pmx_int(bone_index_size);
                auto w = br.read_float4();
                bones_index.set(i, {a, b, c, d});
                bones_weight.set(i, {w[0], w[1], w[2], w[3]});
            }
            else if (weight_type == 3) {
                float a = br.read_pmx_int(bone_index_size);
                float b = br.read_pmx_int(bone_index_size);
                auto w = br.read_LE<float>();
                br.read_float3();
                br.read_float3();
                br.read_float3();
                bones_index.set(i, {a, b, -1, -1});
                bones_weight.set(i, {w, 1-w, -1, -1});
            }
            auto edge_scale = br.read_LE<float>();
            edge_scale_attrs.set(start_ptoff + i, edge_scale);
        }
    }
    {
        auto nvtxs = br.read_LE<uint32_t>();
        GA_Offset start_vtxoff;
        detail->appendPrimitivesAndVertices(GA_PRIMPOLY, nvtxs / 3, 3, start_vtxoff, true);
        for (auto i = 0; i < nvtxs; i++) {
            auto j = br.read_pmx_int(vertex_index_size);
            detail->setVertexPoint(start_vtxoff + i, start_ptoff + j);
        }
    }
    {        
        std::vector<std::string> textures;
        auto count = br.read_LE<uint32_t>();
        for (auto i = 0; i < count; i++) {
            auto name = br.read_pmx_string();
            textures.push_back(name);
        }
        setUserData(detail, "tex_names", textures);
    }
    {
        GA_RWHandleS mat_names(detail->addStringTuple(GA_ATTRIB_PRIMITIVE , "material_name", 1));
        auto start = 0;
        auto count = br.read_LE<uint32_t>();
        for (auto i = 0; i < count; i++) {
            auto name = br.read_pmx_string();
            auto name_en = br.read_pmx_string();
            auto diffuse = br.read_float4();
            auto specular = br.read_float4();
            auto ambient = br.read_float3();
            auto draw_flag = br.read_LE<uint8_t>();
            auto edge_color = br.read_float4();
            auto edge_scale = br.read_LE<float>();
            auto tex_index = br.read_pmx_int(texture_index_size);
            auto env_index = br.read_pmx_int(texture_index_size);
            auto env_blend_mode = br.read_LE<uint8_t>();
            auto toon_ref = br.read_LE<uint8_t>();
            if (toon_ref == 0) {
                br.read_pmx_int(texture_index_size);
            } else {
                br.read_LE<uint8_t>();
            }
            auto comment = br.read_pmx_string();
            auto associated_face_count = br.read_LE<int32_t>() / 3;
            {
                UT_StringHolder name_sh = name;
                for (auto i = start; i < start + associated_face_count; i++) {
                    mat_names.set(i, name_sh);
                }
                start += associated_face_count;
            }
        }
    }
    {
        std::vector<std::string> bone_names;
        auto count = br.read_LE<uint32_t>();
        for (auto i = 0; i < count; i++) {
            auto name = br.read_pmx_string();
            bone_names.push_back(name);
            auto name_en = br.read_pmx_string();
            auto pos = br.read_float3();
            auto parent_index = br.read_pmx_int(bone_index_size);
            auto layer = br.read_LE<int32_t>();
            auto bone_flags = BoneFlags(br.read_LE<uint16_t>());
            if (bone_flags.contains(BoneFlags::INDEXED_TAIL_BONE)) {
                br.read_pmx_int(bone_index_size);
            } else {
                br.read_float3();
            };
            if (bone_flags.contains(BoneFlags::INHERIT_ROTATION) || bone_flags.contains(BoneFlags::INHERIT_TRANSLATION)) {
                auto parent_index = br.read_pmx_int(bone_index_size);
                auto affect = br.read_LE<float>();
            }
            if (bone_flags.contains(BoneFlags::FIXED_AXIS)) {
                br.read_float3();
            }
            if (bone_flags.contains(BoneFlags::LOCAL_AXIS)) {
                br.read_float3();
                br.read_float3();
            }
            if (bone_flags.contains(BoneFlags::EXTERNAL_PARENT)) {
                br.read_pmx_int(bone_index_size);
            }
            if (bone_flags.contains(BoneFlags::IK)) {
                auto effector = br.read_pmx_int(bone_index_size);
                auto loop_count = br.read_LE<int32_t>();
                auto limit_angle = br.read_LE<float>();
                auto link_count = br.read_LE<int32_t>();
                for (auto i = 0; i < link_count; i++) {
                    auto bone = br.read_pmx_int(bone_index_size);
                    if (br.read_LE<uint8_t>() == 1) {
                        auto limit_min = br.read_float3();
                        auto limit_max = br.read_float3();
                    }
                }
            }
        }
        setUserData(detail, "bone_names", bone_names);
    }
}

static void read_bones(
    BinaryReader &br
    , GU_Detail *detail
    , float scale
    , uint8_t appendix_uv
    , uint8_t vertex_index_size
    , uint8_t texture_index_size
    , uint8_t material_index_size
    , uint8_t bone_index_size
    , uint8_t morph_index_size
    , uint8_t rigidbody_index_size
) {
    {
        auto npoints = br.read_LE<uint32_t>();

        for (auto i = 0; i < npoints; i++) {
            auto pos = br.read_float3();
            auto nrm = br.read_float3();
            auto uv = br.read_float2();
            for (auto j = 0; j < appendix_uv; j++) {
                br.read_float4();
            }
            auto weight_type = br.read_LE<uint8_t>();
            if (weight_type == 0) {
                br.read_pmx_int(bone_index_size);
            }
            else if (weight_type == 1) {
                auto a = br.read_pmx_int(bone_index_size);
                auto b = br.read_pmx_int(bone_index_size);
                auto weight = br.read_LE<float>();
            }
            else if (weight_type == 2 || weight_type == 4) {
                auto a = br.read_pmx_int(bone_index_size);
                auto b = br.read_pmx_int(bone_index_size);
                auto c = br.read_pmx_int(bone_index_size);
                auto d = br.read_pmx_int(bone_index_size);
                auto weight = br.read_float4();
            }
            else if (weight_type == 3) {
                auto a = br.read_pmx_int(bone_index_size);
                auto b = br.read_pmx_int(bone_index_size);
                auto weight = br.read_LE<float>();
                br.read_float3();
                br.read_float3();
                br.read_float3();
            }
            auto edge_scale = br.read_LE<float>();
        }
    }
    {
        auto nvtxs = br.read_LE<uint32_t>();
        GA_Offset start_vtxoff;
        for (auto i = 0; i < nvtxs; i++) {
            auto j = br.read_pmx_int(vertex_index_size);
        }
    }
    {        
        auto count = br.read_LE<uint32_t>();
        for (auto i = 0; i < count; i++) {
            auto name = br.read_pmx_string();
        }
    }
    {
        auto count = br.read_LE<uint32_t>();
        for (auto i = 0; i < count; i++) {
            auto name = br.read_pmx_string();
            auto name_en = br.read_pmx_string();
            auto diffuse = br.read_float4();
            auto specular = br.read_float4();
            auto ambient = br.read_float3();
            auto draw_flag = br.read_LE<uint8_t>();
            auto edge_color = br.read_float4();
            auto edge_scale = br.read_LE<float>();
            auto tex_index = br.read_pmx_int(texture_index_size);
            auto env_index = br.read_pmx_int(texture_index_size);
            auto env_blend_mode = br.read_LE<uint8_t>();
            auto toon_ref = br.read_LE<uint8_t>();
            if (toon_ref == 0) {
                br.read_pmx_int(texture_index_size);
            } else {
                br.read_LE<uint8_t>();
            }
            auto comment = br.read_pmx_string();
            auto associated_face_count = br.read_LE<int32_t>() / 3;
        }
    }
    {
        auto count = br.read_LE<uint32_t>();
        GA_Offset start_ptoff = detail->appendPointBlock(count);
        GA_RWHandleS bone_names(detail->addStringTuple(GA_ATTRIB_POINT , "name", 1));
        GA_RWHandleM3 attr_transform = GA_RWHandleM3(detail->addFloatTuple(GA_ATTRIB_POINT, "transform", 9));

        std::vector<int> buffer;
        for (auto i = 0; i < count; i++) {
            UT_Matrix3 matrix;
            matrix.identity();
            attr_transform.set(i, matrix);
            auto name = br.read_pmx_string();
            bone_names.set(i, name);
            auto name_en = br.read_pmx_string();
            auto pos = br.read_float3();
            detail->setPos3(start_ptoff + i, UT_Vector3(pos[0], pos[1], -pos[2]) * scale);
            auto parent_index = br.read_pmx_int(bone_index_size);
            if (parent_index > 0) {
                buffer.push_back(parent_index);
                buffer.push_back(i);
            }
            auto layer = br.read_LE<int32_t>();
            auto bone_flags = BoneFlags(br.read_LE<uint16_t>());
            if (bone_flags.contains(BoneFlags::INDEXED_TAIL_BONE)) {
                br.read_pmx_int(bone_index_size);
            } else {
                br.read_float3();
            };
            if (bone_flags.contains(BoneFlags::INHERIT_ROTATION) || bone_flags.contains(BoneFlags::INHERIT_TRANSLATION)) {
                auto parent_index = br.read_pmx_int(bone_index_size);
                auto affect = br.read_LE<float>();
            }
            if (bone_flags.contains(BoneFlags::FIXED_AXIS)) {
                br.read_float3();
            }
            if (bone_flags.contains(BoneFlags::LOCAL_AXIS)) {
                br.read_float3();
                br.read_float3();
            }
            if (bone_flags.contains(BoneFlags::EXTERNAL_PARENT)) {
                br.read_pmx_int(bone_index_size);
            }
            if (bone_flags.contains(BoneFlags::IK)) {
                auto effector = br.read_pmx_int(bone_index_size);
                auto loop_count = br.read_LE<int32_t>();
                auto limit_angle = br.read_LE<float>();
                auto link_count = br.read_LE<int32_t>();
                for (auto i = 0; i < link_count; i++) {
                    auto bone = br.read_pmx_int(bone_index_size);
                    if (br.read_LE<uint8_t>() == 1) {
                        auto limit_min = br.read_float3();
                        auto limit_max = br.read_float3();
                    }
                }
            }
        }

        GA_Offset start_vtxoff;
        detail->appendPrimitivesAndVertices(GA_PRIMPOLY, buffer.size() / 2, 2, start_vtxoff, true);
        for (auto i = 0; i < buffer.size(); i++) {
            auto j = buffer[i];
            detail->setVertexPoint(start_vtxoff + i, start_ptoff + j);
        }
    }
}

/// This is the function that does the actual work.
void
SOP_PmxVerb::cook(const SOP_NodeVerb::CookParms &cookparms) const
{
    auto &&sopparms = cookparms.parms<SOP_PmxParms>();
    const SOP_PmxParms::Mode mode = sopparms.getMode();

    float scale = sopparms.getScale();
    GU_Detail *detail = cookparms.gdh().gdpNC();
    detail->clearAndDestroy();

    auto file_path = sopparms.getPmx_path().toStdString();
    auto bin = file_get_binary(file_path);
    auto br = BinaryReader(bin);
    auto [_0, _1, _2, _3] = br.read_LE<std::array<uint8_t, 4>>();
    auto version = br.read_LE<float>();
    br.read_LE<uint8_t>();
    auto utf8 = br.read_LE<uint8_t>() == 1;
    auto appendix_uv = br.read_LE<uint8_t>();
    auto vertex_index_size = br.read_LE<uint8_t>();
    auto texture_index_size = br.read_LE<uint8_t>();
    auto material_index_size = br.read_LE<uint8_t>();
    auto bone_index_size = br.read_LE<uint8_t>();
    auto morph_index_size = br.read_LE<uint8_t>();
    auto rigidbody_index_size = br.read_LE<uint8_t>();
    auto name = br.read_pmx_string();
    auto name_en = br.read_pmx_string();
    auto comment = br.read_pmx_string();
    auto comment_en = br.read_pmx_string();

    setUserData(detail, "file_path", file_path);
    setUserData(detail, "name", name);
    setUserData(detail, "name_en", name_en);
    setUserData(detail, "comment", comment);
    setUserData(detail, "comment_en", comment_en);
    setUserData(detail, "version", version);

    if (mode == SOP_PmxParms::Mode::BONE) {
        read_bones(
            br
            , detail
            , scale
            , appendix_uv
            , vertex_index_size
            , texture_index_size
            , material_index_size
            , bone_index_size
            , morph_index_size
            , rigidbody_index_size
        );
    }
    else {
        read_skin(
            br
            , detail
            , scale
            , appendix_uv
            , vertex_index_size
            , texture_index_size
            , material_index_size
            , bone_index_size
            , morph_index_size
            , rigidbody_index_size
        );
    }

    detail->bumpDataIdsForAddOrRemove(true, true, true);
}
