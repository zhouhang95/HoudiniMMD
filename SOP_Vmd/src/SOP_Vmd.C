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
 * The Vmd SOP
 */

#include "SOP_Vmd.h"
#include "SOP_common.h"

// This is an automatically generated header file based on theDsFile, below,
// to provide SOP_VmdParms, an easy way to access parameter values from
// SOP_VmdVerb::cook with the correct type.
#include "SOP_Vmd.proto.h"

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
const UT_StringHolder SOP_Vmd::theSOPTypeName("mmd_vmd"_sh);

/// newSopOperator is the hook that Houdini grabs from this dll
/// and invokes to register the SOP.  In this case, we add ourselves
/// to the specified operator table.
void
newSopOperator(OP_OperatorTable *table)
{
    table->addOperator(new OP_Operator(
        SOP_Vmd::theSOPTypeName,   // Internal name
        "Vmd",                     // UI name
        SOP_Vmd::myConstructor,    // How to build the SOP
        SOP_Vmd::buildTemplates(), // My parameters
        1,                          // Min # of sources
        1,                          // Max # of sources
        nullptr,                    // Custom local variables (none)
        OP_FLAG_GENERATOR));        // Flag it as generator
}

/// This is a multi-line raw string specifying the parameter interface
/// for this SOP.
static const char *theDsFile = R"THEDSFILE(
{
    name        parameters
    parm {
        name    "vmd_path"
        label   "VMD Path"
        type    file
        default { "D:/IA_Conqueror_light_version.vmd" }
        parmtag { "filechooser_pattern" "*.vmd" }
        parmtag { "filechooser_mode" "read" }
    }
    parm {
        name    "scale"
        label   "Scale"
        type    float
        default { "0.08" }
    }
    parm {
        name    "frame"
        label   "Frame"
        type    integer
        default { "$F" }
    }
}
)THEDSFILE";

PRM_Template*
SOP_Vmd::buildTemplates()
{
    static PRM_TemplateBuilder templ("SOP_Vmd.C"_sh, theDsFile);
    return templ.templates();
}

class SOP_VmdVerb : public SOP_NodeVerb
{
public:
    SOP_VmdVerb() {}
    virtual ~SOP_VmdVerb() {}

    virtual SOP_NodeParms *allocParms() const { return new SOP_VmdParms(); }
    virtual UT_StringHolder name() const { return SOP_Vmd::theSOPTypeName; }

    virtual CookMode cookMode(const SOP_NodeParms *parms) const { return COOK_GENERIC; }

    virtual void cook(const CookParms &cookparms) const;
    
    /// This static data member automatically registers
    /// this verb class at library load time.
    static const SOP_NodeVerb::Register<SOP_VmdVerb> theVerb;
};

// The static member variable definition has to be outside the class definition.
// The declaration is inside the class.
const SOP_NodeVerb::Register<SOP_VmdVerb> SOP_VmdVerb::theVerb;

const SOP_NodeVerb *
SOP_Vmd::cookVerb() const 
{ 
    return SOP_VmdVerb::theVerb.get();
}


// Shift-JIS to UTF-8 conversion function
static std::string ShiftJISToUTF8(const std::string shiftJISStr) {
    // Step 1: Convert Shift-JIS to UTF-16
    int wideCharLen = MultiByteToWideChar(932, 0, shiftJISStr.c_str(), -1, NULL, 0);
    if (wideCharLen == 0) {
        return ""; // Conversion failed
    }
    std::wstring wideStr(wideCharLen, 0);
    MultiByteToWideChar(932, 0, shiftJISStr.c_str(), -1, &wideStr[0], wideCharLen);

    // Step 2: Convert UTF-16 to UTF-8
    int utf8Len = WideCharToMultiByte(CP_UTF8, 0, wideStr.c_str(), -1, NULL, 0, NULL, NULL);
    if (utf8Len == 0) {
        return ""; // Conversion failed
    }
    std::string utf8Str(utf8Len, 0);
    WideCharToMultiByte(CP_UTF8, 0, wideStr.c_str(), -1, &utf8Str[0], utf8Len, NULL, NULL);
    if (utf8Str.size() > 0) {
        utf8Str.pop_back();
    }

    return utf8Str;
}

struct BoneFrame {
    glm::vec3 trans;
    glm::quat rot;
};

struct Bezier {
    glm::vec2 p1;
    glm::vec2 p2;
    Bezier() {}

    Bezier(glm::vec4 c) {
        p1 = {c[0], c[1]};
        p2 = {c[2], c[3]};
    }

    glm::vec2 eval(float t) const {
        glm::vec2 res_p1 = p1 * (3.0f * t * (1.0f - t) * (1.0f - t));
        glm::vec2 res_p2 = p2 * (3.0f * t * t * (1.0f - t));
        float i3 = t * t * t;
        return {
            res_p1.x + res_p2.x + i3,
            res_p1.y + res_p2.y + i3
        };
    }

    float find_t_when_x(float x) const {
        const float epsilon = 0.00001f;
        float start = 0.0f;
        float end = 1.0f;
        float t = 0.5f;
        float cur_x = eval(t).x;
        int bezier_max_count = 100;

        while (glm::abs(x - cur_x) > epsilon && bezier_max_count > 0) {
            if (x < cur_x) {
                end = t;
            } else {
                start = t;
            }
            t = (start + end) * 0.5f;
            cur_x = eval(t).x;
            --bezier_max_count;
        }
        return t;
    }

    float find_y_when_x(float x) const {
        float t = find_t_when_x(x);
        return eval(t).y;
    }
};


struct BoneKeyframe {
    uint32_t frame;
    glm::vec3 trans;
    glm::quat rot;
    Bezier txc;
    Bezier tyc;
    Bezier tzc;
    Bezier rc;
    BoneFrame to_bone_frame() {
        BoneFrame bf;
        bf.trans = trans;
        bf.rot = rot;
        return bf;
    }
    bool is_zero() {
        return trans[0] == 0 && trans[1] == 0 && trans[2] == 0
            && rot[0] == 0 && rot[1] == 0 && rot[2] == 0 && rot[3] == 1;
    }
    BoneFrame lerp(BoneKeyframe &next_keyframe, float t) {
        BoneFrame bf;
        auto &n = next_keyframe;
        float t_tx = n.txc.find_y_when_x(t);
        float t_ty = n.tyc.find_y_when_x(t);
        float t_tz = n.tzc.find_y_when_x(t);
        float t_r  = n.rc .find_y_when_x(t);
        bf.trans = {
            glm::lerp(trans.x, n.trans.x, t_tx),
            glm::lerp(trans.y, n.trans.y, t_ty),
            glm::lerp(trans.z, n.trans.z, t_tz),
        };
        bf.rot = glm::slerp(rot, n.rot, t_r);
        return bf;
    }
};

static glm::vec4 read_bezier_control_point_pair4(BinaryReader &br) {
    float x = float(br.read_LE<uint32_t>() & 0xFF) / 127.0;
    float y = float(br.read_LE<uint32_t>() & 0xFF) / 127.0;
    float z = float(br.read_LE<uint32_t>() & 0xFF) / 127.0;
    float w = float(br.read_LE<uint32_t>() & 0xFF) / 127.0;
    return {x, y, z, w};
}

static std::map<std::string, BoneFrame> read_anim(BinaryReader &br, GU_Detail *detail, float scale, exint currentFrame) {
    auto header = br.read_string(30);
    std::string name = br.read_string(header[24] == '2'? 20 : 10);
    name = ShiftJISToUTF8(name);

    setUserData(detail, "vmd_name", name);
    setUserData(detail, "currentFrame", currentFrame);

    auto count = br.read_LE<uint32_t>();
    std::map<std::string, std::vector<BoneKeyframe>> bone_keyframes;
    for (auto i = 0; i < count; i++) {
        BoneKeyframe bk;
        auto name = br.read_string(15);
        name = ShiftJISToUTF8(name);
        bk.frame = br.read_LE<uint32_t>();
        bk.trans = br.read_float3() * glm::vec3(1, 1, -1) * scale;
        bk.rot = br.read_quat();
        bk.rot.x *= -1;
        bk.rot.y *= -1;
        bk.txc = Bezier(read_bezier_control_point_pair4(br));
        bk.tyc = Bezier(read_bezier_control_point_pair4(br));
        bk.tzc = Bezier(read_bezier_control_point_pair4(br));
        bk.rc  = Bezier(read_bezier_control_point_pair4(br));
        bone_keyframes[name].push_back(bk);
    }
    {
        std::vector<std::string> need_remove_names;
        for (auto &[name, kfs]: bone_keyframes) {
            bool need_remove = true;
            for (auto &kf: kfs) {
                if (kf.is_zero() == false) {
                    need_remove = false;
                    break;
                }
            }
            if (need_remove) {
                need_remove_names.push_back(name);
            }
            else {
                std::sort(kfs.begin(), kfs.end(), [](const auto& a, const auto& b) {
                    return a.frame < b.frame;
                });
            }
        }
        for (auto &name: need_remove_names) {
            bone_keyframes.erase(name);
        }
    }
    std::map<std::string, BoneFrame> cur_bone_frames;
    for (auto &[name, kfs]: bone_keyframes) {
        size_t cur_index = 0;
        for (size_t i = 0; i < kfs.size(); ++i) {
            if (kfs[i].frame <= currentFrame) {
                cur_index = i;
            } else {
                break;
            }
        }

        auto& kf = kfs[cur_index];
        BoneFrame result_frame;

        if (kf.frame == currentFrame || cur_index == kfs.size() - 1) {
            result_frame = kf.to_bone_frame();
        } else {
            auto& nkf = kfs[cur_index + 1];
            float t = static_cast<float>(currentFrame - kf.frame) / (nkf.frame - kf.frame);
            result_frame = kf.lerp(nkf, t);
        }
        cur_bone_frames[name] = result_frame;
    }
    return cur_bone_frames;
}


static UT_Matrix3 glmmat3_to_utmat3(glm::mat3 matrix) {
    UT_Matrix3 mat;
    mat[0] = {matrix[0][0], matrix[0][1], matrix[0][2]};
    mat[1] = {matrix[1][0], matrix[1][1], matrix[1][2]};
    mat[2] = {matrix[2][0], matrix[2][1], matrix[2][2]};
    return mat;
}

static glm::mat3 utmat3_to_glmmat3(UT_Matrix3 matrix) {
    glm::mat3 mat;
    mat[0] = {matrix[0][0], matrix[0][1], matrix[0][2]};
    mat[1] = {matrix[1][0], matrix[1][1], matrix[1][2]};
    mat[2] = {matrix[2][0], matrix[2][1], matrix[2][2]};
    return mat;
}
static std::vector<glm::mat4> getBoneMatrix(const GU_Detail *detail) {
    std::vector<glm::mat4> matrixs;
    GA_ROHandleM3 attr_transform = GA_ROHandleM3(detail->findFloatTuple(GA_ATTRIB_POINT, "transform", 9));
    for (GA_Iterator it(detail->getPointRange()); !it.atEnd(); ++it) {
        UT_Vector3 pos = detail->getPos3(*it);
        UT_Matrix3 matrix = attr_transform.get(*it);
        glm::mat4 mat;
        mat[0] = {matrix[0][0], matrix[0][1], matrix[0][2], 0};
        mat[1] = {matrix[1][0], matrix[1][1], matrix[1][2], 0};
        mat[2] = {matrix[2][0], matrix[2][1], matrix[2][2], 0};
        mat[3] = {pos[0], pos[1], pos[2], 1};
        matrixs.push_back(mat);
    }
    return matrixs;
}
static std::vector<glm::mat4> getInvertedBoneMatrix(const GU_Detail *detail) {
    std::vector<glm::mat4> inv_matrixs;
    auto matrixs = getBoneMatrix(detail);
    for (auto i = 0; i < matrixs.size(); i++) {
        auto m = matrixs[i];
        auto inv_m = glm::inverse(m);
        inv_matrixs.push_back(inv_m);
    }
    return inv_matrixs;
}

static UT_Vector3 transform_pos(glm::mat4 &transform, UT_Vector3 pos) {
    auto p = transform * glm::vec4(pos[0], pos[1], pos[2], 1);
    return {p.x, p.y, p.z};
}
static UT_Vector3 transform_nrm(glm::mat4 &transform, UT_Vector3 pos) {
    auto p = glm::transpose(glm::inverse(transform)) * glm::vec4(pos[0], pos[1], pos[2], 0);
    return {p.x, p.y, p.z};
}

/// This is the function that does the actual work.
void
SOP_VmdVerb::cook(const SOP_NodeVerb::CookParms &cookparms) const
{
    auto &&sopparms = cookparms.parms<SOP_VmdParms>();

    float scale = sopparms.getScale();
    GU_Detail *detail = cookparms.gdh().gdpNC();
    detail->clearAndDestroy();
    auto file_path = sopparms.getVmd_path().toStdString();
    setUserData(detail, "file_path", file_path);
    auto bin = file_get_binary(file_path);
    auto br = BinaryReader(bin);
    auto currentFrame = sopparms.getFrame();

    if (cookparms.hasInput(0)) {
        const GU_Detail *input_0 = cookparms.inputGeo(0);
        detail->duplicate(*input_0);
        std::vector<glm::vec3> poss;
        std::vector<std::string> bone_names;
        GA_ROHandleS bone_names_attr(input_0->findStringTuple(GA_ATTRIB_POINT , "name", 1));
        for (GA_Iterator it(input_0->getPointRange()); !it.atEnd(); ++it) {
            UT_Vector3 pos = input_0->getPos3(*it);
            poss.emplace_back(pos.x(), pos.y(), pos.z());
            bone_names.push_back(bone_names_attr.get(*it).toStdString());
        }
    }
    std::map<std::string, BoneFrame> anim = read_anim(br, detail, scale, currentFrame);

    std::map<exint, exint> bone_connects;
    std::vector<exint> vertex;
    for (GA_Iterator it(detail->getPrimitiveRange()); !it.atEnd(); ++it) {
        const GA_Primitive* prim = detail->getPrimitive(*it);
        exint p = prim->getPointOffset(0);
        exint c = prim->getPointOffset(1);
        bone_connects[c] = p;
    }
    auto transforms    = getBoneMatrix(detail);
    auto transformsInv = getInvertedBoneMatrix(detail);
    std::map<int, glm::mat4> cache;
    exint bi = 0;
    GA_ROHandleS bone_names(detail->findStringTuple(GA_ATTRIB_POINT , "name", 1));
    GA_RWHandleM3 attr_transform(detail->findFloatTuple(GA_ATTRIB_POINT, "transform", 9));
    for (GA_Iterator it(detail->getPointRange()); !it.atEnd(); ++it) {
        glm::mat4 transform = glm::mat4(1.0f);
        std::string bone_name = bone_names.get(*it).toStdString();
        if (anim.count(bone_name)) {
            auto trans = anim[bone_name];
            transform = glm::translate(trans.trans) * glm::toMat4(trans.rot);
            transform = transforms[bi] * transform * transformsInv[bi];
        }

        if (bone_connects.count(bi)) {
            transform = cache[bone_connects[bi]] * transform;
        }

        cache[bi] = transform;
        UT_Vector3 pos = transform_pos(transform, detail->getPos3(*it));
        detail->setPos3(*it, pos);

        UT_Matrix3 pose = attr_transform.get(*it);
        pose = glmmat3_to_utmat3(glm::mat3(transform) * utmat3_to_glmmat3(pose));
        attr_transform.set(*it, pose);
        bi += 1;
    }
    detail->bumpDataIdsForAddOrRemove(true, true, true);
}
