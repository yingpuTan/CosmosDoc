using System;
using System.Runtime.InteropServices;
using System.Runtime.Serialization;
using System.Text;
using System.Text.Json.Nodes;
using System.Threading;

//using System.Text.Encoding.CodePages;
using Newtonsoft.Json;
using Newtonsoft.Json.Linq;

namespace RpcWrapper
{
    public class CSharpRpcWrapper
    {
        // 定义 C++ 枚举类型
        public enum RET_CALL
        {
            Exception = -3,
            Sendfail = -2,
            Timeout = -1,
            Pipenull = 0,
            Ok = 1
        }

        /// <summary>
        /// 消息调用类型枚举，定义不同范围的消息发送方式。
        /// </summary>
        public enum InvokeType
        {
            /// <summary>
            /// 给具体实例发送消息，直接针对某个特定对象。
            /// </summary>
            Instance,

            /// <summary>
            /// 给某类的所有组件实例发送消息，作用于该类型的全部实例。
            /// </summary>
            Group,

            /// <summary>
            /// 给全局范围发送消息，所有监听该消息的对象都会收到。
            /// </summary>
            Global
        }

        public class RpcRequest
        {
            public string id = "";
            public string method;
            public JToken param;
        }

        public class RpcResponse
        {
            public string id = "";
            public int code = -1;
            public JToken error = JValue.CreateNull();
            public JToken result = JValue.CreateNull();
        }

        public class RpcPush
        {
            public string topic= "";
            public JToken param;
        }

        public delegate RpcResponse OnInvokeDelegate(RpcRequest param);
        public delegate void OnNotifyDelegate(RpcRequest param);
        public delegate void OnSubscribeDelegate(RpcRequest param);
        public delegate void OnPushDelegate(RpcPush param);
        public delegate void OnFreeVoidPtrDelegate(IntPtr param);

        public event OnInvokeDelegate OnInvoke;
        public event OnNotifyDelegate OnNotify;
        public event OnSubscribeDelegate OnSubscribe;
        public event OnPushDelegate OnPush;
        public event OnFreeVoidPtrDelegate OnFreeVoidPtr;

        // 定义 C++ 委托类型
        [UnmanagedFunctionPointer(CallingConvention.Cdecl)]
        public delegate IntPtr CPPOnInvokeDelegate(IntPtr param, int size);

        [UnmanagedFunctionPointer(CallingConvention.Cdecl)]
        public delegate void CPPOnNotifyDelegate(IntPtr param, int size);

        [UnmanagedFunctionPointer(CallingConvention.Cdecl)]
        public delegate void CPPOnSubscribeDelegate(IntPtr param, int size);

        [UnmanagedFunctionPointer(CallingConvention.Cdecl)]
        public delegate void CPPOnPushDelegate(IntPtr param, int size);

        [UnmanagedFunctionPointer(CallingConvention.Cdecl)]
        public delegate void CPPOnFreeVoidPtrDelegate(IntPtr param);

        // 定义回调函数
        private static CPPOnInvokeDelegate cpp_onInvoke;
        private static CPPOnNotifyDelegate cpp_onNotify;
        private static CPPOnSubscribeDelegate cpp_onSubscribe;
        private static CPPOnPushDelegate cpp_onPush;
        private static CPPOnFreeVoidPtrDelegate cpp_OnFreeVoidPtr;

        // 导入 C++ 函数
        [DllImport("PipeClient", CallingConvention = CallingConvention.Cdecl)]
        public static extern bool InitClient(IntPtr pipename, int pipename_size, IntPtr logpath, int log_path_size, int protocolLevel, double del_log_cycle, bool bdetaillog);

        [DllImport("PipeClient", CallingConvention = CallingConvention.Cdecl)]
        public static extern int Invoke(IntPtr _in, ref IntPtr _out, out int _outsize, int timeout = 30000);

        [DllImport("PipeClient", CallingConvention = CallingConvention.Cdecl)]
        public static extern int Notify(IntPtr _in);

        [DllImport("PipeClient", CallingConvention = CallingConvention.Cdecl)]
        public static extern int Subscribe(IntPtr _in);

        [DllImport("PipeClient", CallingConvention = CallingConvention.Cdecl)]
        public static extern int Push(IntPtr _in);

        [DllImport("PipeClient", CallingConvention = CallingConvention.Cdecl)]
        public static extern IntPtr CreateRpcRequest(string id, string method, IntPtr param);

        [DllImport("PipeClient", CallingConvention = CallingConvention.Cdecl)]
        public static extern IntPtr CreateRpcPush(string topic, IntPtr param);

        [DllImport("PipeClient", CallingConvention = CallingConvention.Cdecl)]
        public static extern void FreeRpcAllocMemory(IntPtr _in);

        [DllImport("PipeClient", CallingConvention = CallingConvention.Cdecl)]
        public static extern void Register(CPPOnInvokeDelegate invoke, CPPOnNotifyDelegate notify, CPPOnSubscribeDelegate subscribe, CPPOnPushDelegate push, CPPOnFreeVoidPtrDelegate freevoidptr);

        [DllImport("PipeClient", CallingConvention = CallingConvention.Cdecl)]
        public static extern void ExitApp();

        public CSharpRpcWrapper()
        {
            Encoding.RegisterProvider(CodePagesEncodingProvider.Instance);
        }

        public bool InitClient(string pipename, string logpath, double del_log_cycle = 3, bool bdetaillog = false)
        {
            IntPtr pipenamePtr = Marshal.StringToCoTaskMemAnsi(pipename);
            IntPtr logpathPtr = Marshal.StringToCoTaskMemAnsi(logpath);

            bool bRet = InitClient(pipenamePtr, pipename.Length, logpathPtr, logpath.Length, 2, del_log_cycle, bdetaillog);
            Marshal.FreeCoTaskMem(pipenamePtr);
            Marshal.FreeCoTaskMem(logpathPtr);
            return bRet;
        }

        public void RegisterCallback()
        {
            // 注册回调函数
            cpp_onInvoke = new CPPOnInvokeDelegate(On_Invoke);
            cpp_onNotify = new CPPOnNotifyDelegate(On_Notify);
            cpp_onPush = new CPPOnPushDelegate(On_Push);
            cpp_onSubscribe = new CPPOnSubscribeDelegate(On_Subscribe);
            cpp_OnFreeVoidPtr = new CPPOnFreeVoidPtrDelegate(On_FreeBusiparam);

            Register(cpp_onInvoke, cpp_onNotify, cpp_onSubscribe, cpp_onPush, cpp_OnFreeVoidPtr);
        }

        private IntPtr On_Invoke(IntPtr param, int size)
        {
            try{
                RpcRequest request = DeserializeRpcRequest(param, size);

                RpcResponse resp = OnInvoke?.Invoke(request) ?? default(RpcResponse);

                string msg = JsonConvert.SerializeObject(resp);
                IntPtr _out = Marshal.StringToHGlobalAnsi(msg);
                return _out;
            }
            catch (Exception ex) {

                return IntPtr.Zero;
            }            
        }        

        private void On_Notify(IntPtr param, int size)
        {
            RpcRequest request = DeserializeRpcRequest(param, size);            
            OnNotify?.Invoke(request);            
        }

        private void On_Subscribe(IntPtr param, int size)
        {
            RpcRequest request = DeserializeRpcRequest(param, size);
            OnSubscribe?.Invoke(request);
        }

        private void On_Push(IntPtr param, int size)
        {
            RpcPush push = DeserializeRpcPush(param, size);
            OnPush?.Invoke(push);            
        }

        private void On_FreeBusiparam(IntPtr param)
        {
            Marshal.FreeHGlobal(param);
        }

        public int Invoke(RpcRequest request, out RpcResponse response, int timeout = 30000)
        {
            IntPtr _in = CreatePpcPtrByRequest(request);

            IntPtr _out = IntPtr.Zero;
            int _outsize = 0;
            int ret = Invoke(_in, ref _out, out _outsize, timeout);
            FreeRpcAllocMemory(_in);

            response = DeserializeRpcResponse(_out, _outsize);
            FreeRpcAllocMemory(_out);
            return ret;
        }

        public async Task<(int ret, RpcResponse response)> InvokeAsync(RpcRequest request, int timeout = 30000)
        {
            RpcResponse response = new RpcResponse();
            int ret = await Task.Run(() => Invoke(request, out response, timeout));

            return (ret, response);
        }      

        public int Notify(RpcRequest request)
        {
            IntPtr _in = CreatePpcPtrByRequest(request);

            int ret = Notify(_in);
            FreeRpcAllocMemory(_in);
            return ret;
        }

        /// <summary>
        /// 向组件发送请求
        /// </summary>
        /// <param name="Instanceid">发送对象，根据type填入不同的参数</param>
        /// <param name="type">发送对象类型</param>
        /// <param name="parameter">发送内容</param>
        /// <param name="currentPage">是否只发送给当前页面</param>
        public async Task<(int ret, RpcResponse response)> InvokeWidget(string Instanceid, InvokeType type, RpcRequest parameter, bool currentPage = false)
        {
            var widgetPackage = PackWidget(Instanceid, type, parameter, currentPage);
            return await InvokeAsync(parameter);
        }

        /// <summary>
        /// 发送消息给组件
        /// </summary>
        /// <param name="Instanceid">发送对象，根据type填入不同的参数</param>
        /// <param name="type">发送对象类型</param>
        /// <param name="parameter">发送内容</param>
        /// <param name="currentPage">是否只发送给当前页面</param>
        public int NotifyWidget(string Instanceid, InvokeType type, RpcRequest parameter, bool currentPage = false)
        {
            var widgetPackage = PackWidget(Instanceid, type, parameter, currentPage);
            return Notify(widgetPackage);
        }

        private RpcRequest PackWidget(string Instanceid, InvokeType type, RpcRequest parameter, bool currentPage = false)
        {
            JObject widgetParam = new JObject();
            widgetParam["method"] = parameter.method;
            widgetParam["type"] = (int)type;
            widgetParam["currentPage"] = currentPage;
            widgetParam["instanceid"] = Instanceid;

            if (parameter.param is null)
            {
                parameter.param = new JObject();
            }
            parameter.param["Cosmos:WidgetComunication:Invoke"] = widgetParam;
            parameter.method = "Cosmos:WidgetComunication:Invoke";

            return parameter;
        }
        public int Subscribe(RpcRequest request)
        {
            IntPtr _in = CreatePpcPtrByRequest(request);
            int ret = Subscribe(_in);
            FreeRpcAllocMemory(_in);
            return ret;
        }

        private IntPtr CreatePpcPtrByRequest(RpcRequest request)
        {
            // 获取 GBK 编码
            Encoding gbkEncoding = Encoding.GetEncoding("GBK");

            // 转换为 GBK 编码的字节数组
            string input = request.param?.ToString() ?? "{}";
            byte[] gbkBytes = gbkEncoding.GetBytes(input);

            // 分配内存并复制字节数组
            IntPtr ptr = Marshal.AllocHGlobal(gbkBytes.Length);
            Marshal.Copy(gbkBytes, 0, ptr, gbkBytes.Length);

            IntPtr _out = CreateRpcRequest(request.id, request.method, ptr);

            // 释放内存
            Marshal.FreeHGlobal(ptr);

            return _out;
        }

        public int Push(RpcPush push)
        {
            // 获取 GBK 编码
            Encoding gbkEncoding = Encoding.GetEncoding("GBK");

            // 转换为 GBK 编码的字节数组
            string input = push.param?.ToString() ?? "{}";
            byte[] gbkBytes = gbkEncoding.GetBytes(input);

            // 分配内存并复制字节数组
            IntPtr ptr = Marshal.AllocHGlobal(gbkBytes.Length);
            Marshal.Copy(gbkBytes, 0, ptr, gbkBytes.Length);

            IntPtr _in = CreateRpcPush(push.topic, ptr);

            // 释放内存
            Marshal.FreeHGlobal(ptr);

            int ret = Push(_in);
            FreeRpcAllocMemory(_in);
            return ret;
        }

        public void Exit()
        {
            ExitApp();
        }

        private RpcRequest DeserializeRpcRequest(IntPtr param, int size)
        {
            RpcRequest request = new RpcRequest();
            if (param != IntPtr.Zero)
            {
                // 将 IntPtr 转换为字节数组
                byte[] gbkBytes = new byte[size];
                Marshal.Copy(param, gbkBytes, 0, size);

                // 将 GBK 字节数组解码为 UTF-8 字符串
                string gbkString = Encoding.GetEncoding("GBK").GetString(gbkBytes);
                string utf8String = Encoding.UTF8.GetString(Encoding.Convert(Encoding.GetEncoding("GBK"), Encoding.UTF8, gbkBytes));

                JObject jt = JObject.Parse(utf8String);
                if (jt.ContainsKey("id") && jt["id"] != null)
                {
                    request.id = (string)jt["id"];
                }

                if (jt.ContainsKey("method") && jt["method"] != null)
                {
                    request.method = (string)jt["method"];
                }

                if (jt.ContainsKey("param"))
                {
                    request.param = jt["param"];
                }
            }
            else
            {
                // 处理 param 为 null 的情况
            }            

            return request;
        }

        private RpcResponse DeserializeRpcResponse(IntPtr param, int size)
        {
            RpcResponse resp = new RpcResponse();
            if (param != IntPtr.Zero)
            {
                // 将 IntPtr 转换为字节数组
                byte[] gbkBytes = new byte[size];
                Marshal.Copy(param, gbkBytes, 0, size);

                // 将 GBK 字节数组解码为 UTF-8 字符串
                string gbkString = Encoding.GetEncoding("GBK").GetString(gbkBytes);
                string utf8String = Encoding.UTF8.GetString(Encoding.Convert(Encoding.GetEncoding("GBK"), Encoding.UTF8, gbkBytes));

                var jt = JObject.Parse(utf8String);
                if (jt.ContainsKey("id") && jt["id"] != null)
                {
                    resp.id = (string)jt["id"];
                }

                if (jt.ContainsKey("code") && jt["code"] != null)
                {
                    resp.code = (Int32)jt["code"];
                }

                if (jt.ContainsKey("error"))
                {
                    resp.error = jt["error"];
                }

                if (jt.ContainsKey("result"))
                {
                    resp.result = jt["result"];
                }
            }            

            return resp;
        }

        private RpcPush DeserializeRpcPush(IntPtr param, int size)
        {
            RpcPush push = new RpcPush();
            if (param != IntPtr.Zero)
            {
                // 将 IntPtr 转换为字节数组
                byte[] gbkBytes = new byte[size];
                Marshal.Copy(param, gbkBytes, 0, size);

                // 将 GBK 字节数组解码为 UTF-8 字符串
                string gbkString = Encoding.GetEncoding("GBK").GetString(gbkBytes);
                string utf8String = Encoding.UTF8.GetString(Encoding.Convert(Encoding.GetEncoding("GBK"), Encoding.UTF8, gbkBytes));

                JObject jt = JObject.Parse(utf8String);
                if (jt.ContainsKey("topic") && jt["topic"] != null)
                {
                    push.topic = (string)jt["topic"];
                }

                if (jt.ContainsKey("param"))
                {
                    push.param = jt["param"];
                }
            }            

            return push;
        }
    }
}
