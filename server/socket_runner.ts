// deno run --allow-all socket_runner.ts

const PROMPTS: Record<string, string> = {
  german: "ADD SOME GERMAN LOOKING WORDS FROM TIME TO TIME.",
  russian: "IMPORTANT: WRITE ONLY IN RUSSIAN BUT IN LATIN CHARACTERS.",
};

const T100_ADDITIONAL_PROMPT = PROMPTS[Deno.args[2] || ""] || "";

if (T100_ADDITIONAL_PROMPT){
  console.log("Additional prompt: " + T100_ADDITIONAL_PROMPT)
}

const conn = await Deno.connect({ hostname: "192.168.10.124", port: 1337 });

const proc = new Deno.Command("python3", {
  args: ["llm_core.py"],
  cwd: import.meta.dirname,
  stdin: "piped",
  stdout: "piped",
  env: {
    ...Deno.env.toObject(),
    T100_ADDITIONAL_PROMPT,
    "PYTHONUNBUFFERED": "1"
  },
}).spawn();

conn.readable.pipeTo(proc.stdin);
proc.stdout.pipeTo(conn.writable);
