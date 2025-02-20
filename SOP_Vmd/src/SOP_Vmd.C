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
        2,                          // Max # of sources
        nullptr,                    // Custom local variables (none)
        OP_FLAG_GENERATOR));        // Flag it as generator
}

/// This is a multi-line raw string specifying the parameter interface
/// for this SOP.
static const char *theDsFile = R"THEDSFILE(
{
    name        parameters
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
static BoneKeyframe get_bonekeyframe(const GU_Detail *vmd, int index) {
    auto frame_attr = GA_ROHandleI( vmd->findIntTuple(GA_ATTRIB_POINT, "frame", 1));
    auto rot_attr   = GA_ROHandleQ( vmd->findFloatTuple(GA_ATTRIB_POINT, "rot", 4));
    auto txc_attr   = GA_ROHandleV4(vmd->findFloatTuple(GA_ATTRIB_POINT, "txc", 4));
    auto tyc_attr   = GA_ROHandleV4(vmd->findFloatTuple(GA_ATTRIB_POINT, "tyc", 4));
    auto tzc_attr   = GA_ROHandleV4(vmd->findFloatTuple(GA_ATTRIB_POINT, "tzc", 4));
    auto rc_attr    = GA_ROHandleV4(vmd->findFloatTuple(GA_ATTRIB_POINT, "rc", 4));
    BoneKeyframe bkf;
    bkf.frame = frame_attr.get(index);
    auto trans = vmd->getPos3(index);
    auto rot = rot_attr.get(index);
    auto txc = txc_attr.get(index);
    auto tyc = tyc_attr.get(index);
    auto tzc = tzc_attr.get(index);
    auto rc  = rc_attr.get(index);
    bkf.trans = glm::vec3(trans[0], trans[1], trans[2]);
    bkf.rot = glm::quat(rot[3], rot[0], rot[1], rot[2]);
    bkf.txc = glm::vec4(txc[0], txc[1], txc[2], txc[3]);
    bkf.tyc = glm::vec4(tyc[0], tyc[1], tyc[2], tyc[3]);
    bkf.tzc = glm::vec4(tzc[0], tzc[1], tzc[2], tzc[3]);
    bkf.rc = glm::vec4(rc[0], rc[1], rc[2], rc[3]);
    return bkf;
}
static std::map<std::string, BoneFrame> calc_cur_anim(GU_Detail *detail, const GU_Detail *vmd, exint currentFrame) {
    std::map<std::string, std::vector<glm::ivec2>> bone_keyframes;
    {
        auto frame_attr = GA_ROHandleI(vmd->findIntTuple(GA_ATTRIB_POINT, "frame", 1));
        auto name_attr = GA_ROHandleS(vmd->findStringTuple(GA_ATTRIB_POINT , "name", 1));
        std::string name;
        std::vector<glm::ivec2> frames;
        int count1 = vmd->getNumPoints();
        for (auto i = 0; i < count1; i++) {
            int frame = frame_attr.get(i);
            if (frame == 0) {
                name = name_attr.get(i).toStdString();
                if (frames.size()) {
                    bone_keyframes[name] = frames;
                }
                frames.clear();
            }
            frames.emplace_back(frame, i);
        }
        if (frames.size()) {
            bone_keyframes[name] = frames;
        }
    }
    std::map<std::string, BoneFrame> cur_bone_frames;
    for (auto &[name, kfs]: bone_keyframes) {
        size_t cur_index = 0;
        for (size_t i = 0; i < kfs.size(); ++i) {
            if (kfs[i][0] <= currentFrame) {
                cur_index = i;
            } else {
                break;
            }
        }

        auto kf = get_bonekeyframe(vmd, kfs[cur_index][1]);
        BoneFrame result_frame;

        if (kf.frame == currentFrame || cur_index == kfs.size() - 1) {
            result_frame = kf.to_bone_frame();
        } else {
            auto nkf = get_bonekeyframe(vmd, kfs[cur_index + 1][1]);
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

    GU_Detail *detail = cookparms.gdh().gdpNC();
    detail->clearAndDestroy();
    auto currentFrame = sopparms.getFrame();

    {
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
    if (cookparms.hasInput(1)) {
        const GU_Detail *vmd = cookparms.inputGeo(1);
        std::map<std::string, BoneFrame> anim = calc_cur_anim(detail, vmd, currentFrame);
    
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
        GA_ROHandleS bone_names(detail->findStringTuple(GA_ATTRIB_POINT , "name", 1));
        GA_RWHandleM3 attr_transform(detail->findFloatTuple(GA_ATTRIB_POINT, "transform", 9));
        GA_ROHandleV4 attr_inherit(detail->addFloatTuple(GA_ATTRIB_POINT, "inherit", 4));
        for (GA_Iterator it(detail->getPointRange()); !it.atEnd(); ++it) {
            exint bi = *it;
            glm::mat4 transform = glm::mat4(1.0f);
            UT_Vector4 inherit = attr_inherit.get(bi);
            if (inherit[1] != 0.0f) {
                std::string bone_name = bone_names.get(int(inherit[0])).toStdString();
                if (anim.count(bone_name)) {
                    auto trans = anim[bone_name];
                    glm::vec3 translate = trans.trans * inherit[1] * inherit[3];
                    glm::quat rotation = glm::slerp({1, 0, 0, 0}, trans.rot, inherit[1] * inherit[2]);
                    transform = glm::translate(translate) * glm::toMat4(rotation);
                    transform = transforms[bi] * transform * transformsInv[bi];
                }
            }
            std::string bone_name = bone_names.get(bi).toStdString();
            if (anim.count(bone_name)) {
                auto trans = anim[bone_name];
                transform = glm::translate(trans.trans) * glm::toMat4(trans.rot);
                transform = transforms[bi] * transform * transformsInv[bi];
            }
    
            if (bone_connects.count(bi)) {
                transform = cache[bone_connects[bi]] * transform;
            }
    
            cache[bi] = transform;
            UT_Vector3 pos = transform_pos(transform, detail->getPos3(bi));
            detail->setPos3(bi, pos);
    
            UT_Matrix3 pose = attr_transform.get(bi);
            pose = glmmat3_to_utmat3(glm::mat3(transform) * utmat3_to_glmmat3(pose));
            attr_transform.set(bi, pose);
        }
    }
    detail->bumpDataIdsForAddOrRemove(true, true, true);
}
