using Cosmos.DataAccess.Trade.v1;
using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;

namespace Cosmos.DataAccess.Trade.HighGate.v1
{
    /// <summary>
    /// 数据推理器（例如OpenAI、HexinGPT, Llama2等）
    /// </summary>
    public class AIInferencer : IAIInferencer
    {
        /// <summary>
        /// 可选模型
        /// </summary>
        public String[] AvailableModels { get; }

        /// <summary>
        /// 当前选中的模型
        /// </summary>
        public String SelectedModel { get; set; }

        /// <summary>
        /// 创建推理上下文
        /// </summary>
        /// <param name="modelName"></param>
        /// <param name="vectorContexts"></param>
        /// <returns></returns>
        public IAIInferenceContext CreateContext(String modelName, Int64[] vectorContexts)
        {
            throw new NotImplementedException();
        }

        /// <summary>
        /// 推理
        /// </summary>
        /// <param name="prompt"></param>
        /// <returns></returns>
        public Task<IAIInferenceResult> InferenceAsync(String prompt, IAIInferenceContext inferenceContext)
        {
            throw new NotImplementedException();
        }
    }
    public class AIInferenceContext : IAIInferenceContext
    {
        public String ModelName { get; }
        public Int64[] VectorContexts { get; }
    }

    public class AIInferenceResult : IAIInferenceResult
    {
        /// <summary>
        /// 是否成功
        /// </summary>
        public bool IsSuccess { get; }

        /// <summary>
        /// 回答
        /// </summary>
        public String ReplyMessage { get; }

        /// <summary>
        /// 上下文
        /// </summary>
        public IAIInferenceContext InferenceContext { get; }
    }
}
