# Assumes `pip install llama-cpp-python huggingface-hub`

from zeus_apple_silicon import AppleEnergyMonitor
from llama_cpp import Llama

# (1) Initialize your model
llm = Llama.from_pretrained(
    repo_id="bartowski/Llama-3.2-3B-Instruct-GGUF",
    filename="Llama-3.2-3B-Instruct-Q6_K.gguf",
    n_gpu_layers=-1,
)

# (2) Initialize energy monitor
monitor = AppleEnergyMonitor()

# (3) See how much energy is consumed while generating response
monitor.begin_window("prompt") # START an energy measurement window
output = llm.create_chat_completion(
      messages = [
          {"role": "system", "content": "You are a helpful assistant."},
          {"role": "user", "content": "What makes a good Python library? Answer concisely."}
      ],
)
energy_metrics = monitor.end_window("prompt") # END measurement, get results

print("--- Model Output ---")
print(output["choices"][0]["message"]["content"])

# (4) Print energy usage over the measured window
print("--- Energy ---")
print(energy_metrics)
