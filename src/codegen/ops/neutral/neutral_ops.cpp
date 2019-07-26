#include <codegen/codegen.h>
#include <ir/op_utils.h>
#include <ir/ops/binary.h>
#include <ir/ops/concat.h>
#include <ir/ops/conv2d.h>
#include <ir/ops/dequantize.h>
#include <ir/ops/fake_dequantize.h>
#include <ir/ops/fake_quantize.h>
#include <ir/ops/matmul.h>
#include <ir/ops/quantize.h>
#include <ir/ops/reduce.h>
#include <ir/ops/reduce_window2d.h>
#include <ir/ops/softmax.h>
#include <ir/ops/transpose.h>
#include <runtime/neutral/neutral_ops_body.h>

using namespace nncase;
using namespace nncase::codegen;
using namespace nncase::runtime;
using namespace nncase::ir;
using namespace nncase::runtime::neutral;

namespace nncase
{
namespace codegen
{
    void register_netural_emitters()
    {
        register_emitter(op_binary, [](node &node, codegen_context &context) {
            auto &rnode = static_cast<binary &>(node);
            auto body = std::make_unique<node_body_impl<rop_binary, binary_options>>();

            body->input_a = context.get_allocation(rnode.input_a());
            body->input_b = context.get_allocation(rnode.input_b());
            body->output = context.get_allocation(rnode.output());
            body->binary_op = rnode.binary_op();
            body->in_a_shape = to(rnode.input_a().shape());
            body->in_b_shape = to(rnode.input_b().shape());
            body->out_shape = to(rnode.output().shape());
            body->fused_activation = rnode.fused_activation();

            return body;
        });

        register_emitter(op_concat, [](node &node, codegen_context &context) {
            auto &rnode = static_cast<concat &>(node);
            auto body = std::make_unique<node_body_impl<rop_concat, concat_options>>();

            std::vector<memory_range> inputs;
            for (auto &&in : rnode.inputs())
                inputs.emplace_back(context.get_allocation(in));

            auto elem_size = (uint32_t)runtime::get_bytes(rnode.output().type());
            uint64_t inner_size, outer_size;
            get_concat_params(rnode.output().shape(), elem_size, rnode.axis(), inner_size, outer_size);

            body->output = context.get_allocation(rnode.output());
            body->inner_size = inner_size;
            body->outer_size = outer_size;
            body->inputs_count = (uint32_t)inputs.size();
            body->inputs = inputs;
            body->dims = rnode.concat_dims();

            return body;
        });

        register_emitter(op_conv2d, [](node &node, codegen_context &context) {
            auto &rnode = static_cast<conv2d &>(node);
            auto body = std::make_unique<node_body_impl<rop_conv2d, conv2d_options>>();

            body->input = context.get_allocation(rnode.input());
            body->output = context.get_allocation(rnode.output());
            body->in_shape = to(rnode.input().shape());
            body->groups = rnode.groups();
            body->out_channels = rnode.output_channels();
            body->padding_h = rnode.padding_h();
            body->padding_w = rnode.padding_w();
            body->filter_h = rnode.filter_h();
            body->filter_w = rnode.filter_w();
            body->stride_h = rnode.stride_h();
            body->stride_w = rnode.stride_w();
            body->dilation_h = rnode.dilation_h();
            body->dilation_w = rnode.dilation_w();
            body->fused_activation = rnode.fused_activation();
            body->weights = rnode.weights();
            body->bias = rnode.bias();

            return body;
        });

        register_emitter(op_dequantize, [](node &node, codegen_context &context) {
            auto &rnode = static_cast<dequantize &>(node);
            auto body = std::make_unique<node_body_impl<rop_dequantize, dequantize_options>>();

            body->input = context.get_allocation(rnode.input());
            body->output = context.get_allocation(rnode.output());
            body->quant_param = rnode.quant_param();

            return body;
        });

        register_emitter(op_matmul, [](node &node, codegen_context &context) {
            auto &rnode = static_cast<matmul &>(node);
            auto body = std::make_unique<node_body_impl<rop_matmul, matmul_options>>();

            body->input_a = context.get_allocation(rnode.input_a());
            body->input_b = context.get_allocation(rnode.input_b());
            body->output = context.get_allocation(rnode.output());
            body->a_rows = rnode.input_a().shape()[0];
            body->a_cols = rnode.input_a().shape()[1];
            body->b_cols = rnode.input_b().shape()[1];
            body->fused_activation = rnode.fused_activation();
            body->bias = rnode.bias();

            return body;
        });

        disable_emitter(op_input);
        disable_emitter(op_output);
        disable_emitter(op_constant);

        register_emitter(op_quantize, [](node &node, codegen_context &context) {
            auto &rnode = static_cast<quantize &>(node);
            auto body = std::make_unique<node_body_impl<rop_quantize, quantize_options>>();

            body->input = context.get_allocation(rnode.input());
            body->output = context.get_allocation(rnode.output());
            body->quant_param = rnode.quant_param();

            return body;
        });

        register_emitter(op_reduce, [](node &node, codegen_context &context) {
            auto &rnode = static_cast<reduce &>(node);
            auto body = std::make_unique<node_body_impl<rop_reduce, reduce_options>>();

            auto reduced_shape = get_reduced_shape(rnode.input().shape(), rnode.axis(), true);

            body->input = context.get_allocation(rnode.input());
            body->output = context.get_allocation(rnode.output());
            body->reduce_op = rnode.reduce_op();
            body->in_shape = to(rnode.input().shape());
            body->out_shape = to(reduced_shape);
            body->init_value = rnode.init_value();

            return body;
        });

        register_emitter(op_reduce_window2d, [](node &node, codegen_context &context) {
            auto &rnode = static_cast<reduce_window2d &>(node);
            auto body = std::make_unique<node_body_impl<rop_reduce_window2d, reduce_window2d_options>>();

            body->input = context.get_allocation(rnode.input());
            body->output = context.get_allocation(rnode.output());
            body->reduce_op = rnode.reduce_op();
            body->in_shape = to(rnode.input().shape());
            body->padding_h = rnode.padding_h();
            body->padding_w = rnode.padding_w();
            body->filter_h = rnode.filter_h();
            body->filter_w = rnode.filter_w();
            body->stride_h = rnode.stride_h();
            body->stride_w = rnode.stride_w();
            body->dilation_h = rnode.dilation_h();
            body->dilation_w = rnode.dilation_w();
            body->init_value = rnode.init_value();
            body->fused_activation = rnode.fused_activation();

            return body;
        });

        register_emitter(op_softmax, [](node &node, codegen_context &context) {
            auto &rnode = static_cast<softmax &>(node);
            auto body = std::make_unique<node_body_impl<rop_softmax, softmax_options>>();

            auto in_shape = rnode.input().shape();
            auto inner_size = (int32_t)xt::compute_size(in_shape) / in_shape[0];

            body->input = context.get_allocation(rnode.input());
            body->output = context.get_allocation(rnode.output());
            body->inner_size = inner_size;
            body->outer_size = (int32_t)rnode.input().shape()[0];

            return body;
        });

        register_emitter(op_transpose, [](node &node, codegen_context &context) {
            auto &rnode = static_cast<transpose &>(node);
            auto body = std::make_unique<node_body_impl<rop_transpose, transpose_options>>();

            runtime_shape_t in_shape, perm;
            extend_transpose_shape(rnode.input().shape(), rnode.perm(), in_shape, perm);

            body->input = context.get_allocation(rnode.input());
            body->output = context.get_allocation(rnode.output());
            body->in_shape = in_shape;
            body->perm = perm;

            return body;
        });
    }
}
}