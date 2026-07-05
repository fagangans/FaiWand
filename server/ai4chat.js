// ai4chat.js — klien AI4Chat (API gratis, tanpa API key)
// Diadaptasi dari Faganlenwy/WhatsApp/scrape/Ai4Chat.js untuk dipakai
// oleh gesture_server.js di proyek AI Magic Wand (FaiWand).
import axios from "axios";

export async function ai4Chat(prompt) {
  const url = new URL("https://yw85opafq6.execute-api.us-east-1.amazonaws.com/default/boss_mode_15aug");
  url.search = new URLSearchParams({
    text: prompt,
    country: "Europe",
    user_id: "FaiWand",
  }).toString();

  const response = await axios.get(url.toString(), {
    timeout: 20000,
    headers: {
      "User-Agent": "Mozilla/5.0 (Linux; Android 11; Infinix)",
      Referer: "https://www.ai4chat.co/pages/riddle-generator",
    },
  });

  if (response.status !== 200) throw new Error(`Status ${response.status}`);

  const result = response.data?.trim?.() || null;
  if (!result) throw new Error("Empty AI response");

  return result;
}

// Fallback AI kalau AI4Chat sedang down
async function askPublicAI(prompt) {
  const url = `https://api.fromscratch.web.id/v1/api/ai/publicai?query=${encodeURIComponent(prompt)}`;
  const { data } = await axios.get(url, { timeout: 20000 });
  return data?.data?.response || null;
}

// Panggil AI4Chat & PublicAI bersamaan, pakai jawaban siapa pun yang datang duluan
export function askFastest(prompt) {
  return new Promise((resolve, reject) => {
    const sources = [
      { name: "AI4Chat", fn: () => ai4Chat(prompt) },
      { name: "PublicAI", fn: () => askPublicAI(prompt) },
    ];

    let resolved = false;
    let settledCount = 0;

    sources.forEach(({ name, fn }) => {
      fn()
        .then((result) => {
          if (!resolved && result) {
            resolved = true;
            resolve(result);
          }
        })
        .catch((err) => {
          console.error(`[FaiWand] ${name} gagal:`, err.response?.status || "", err.message);
        })
        .finally(() => {
          settledCount++;
          if (settledCount === sources.length && !resolved) {
            reject(new Error("Semua sumber AI (AI4Chat & PublicAI) sedang tidak merespons."));
          }
        });
    });
  });
}

export default ai4Chat;
