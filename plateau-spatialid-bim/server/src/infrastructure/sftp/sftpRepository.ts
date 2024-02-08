import iconv from "iconv-lite";
import { getSftpClient } from "./sftpClient";

export const listFileName = async (path: string): Promise<string[]> => {
  const client = await getSftpClient();
  const list = await client.list(path);
  return list.map((file) => file.name);
};

export const getFile = async (path: string): Promise<string> => {
  const client = await getSftpClient();
  const buf = (await client.get(path)) as Buffer;
  const str = iconv.decode(buf, "Shift_JIS");
  return str;
};
