using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;

namespace Cosmos.DataAccess.Trade.v1
{
    /// <summary>
    /// 数据推理器（例如OpenAI、HexinGPT, Llama2等）
    /// </summary>
    public interface IAIInferencer
    {
        /// <summary>
        /// 可选模型
        /// </summary>
        String[] AvailableModels { get; }

        /// <summary>
        /// 当前选中的模型
        /// </summary>
        String SelectedModel { get; set; }

        /// <summary>
        /// 创建推理上下文
        /// </summary>
        /// <param name="modelName"></param>
        /// <param name="vectorContexts"></param>
        /// <returns></returns>
        IAIInferenceContext CreateContext(String modelName, Int64[] vectorContexts);

        /// <summary>
        /// 推理
        /// </summary>
        /// <param name="prompt"></param>
        /// <returns></returns>
        Task<IAIInferenceResult> InferenceAsync(String prompt, IAIInferenceContext inferenceContext);
    }
    public interface IAIInferenceContext
    {
        String ModelName { get; }
        Int64[] VectorContexts { get; }
    }

    public interface IAIInferenceResult
    {
        /// <summary>
        /// 是否成功
        /// </summary>
        bool IsSuccess { get; }

        /// <summary>
        /// 回答
        /// </summary>
        String ReplyMessage { get; }

        /// <summary>
        /// 上下文
        /// </summary>
        IAIInferenceContext InferenceContext { get; }
    }
}
