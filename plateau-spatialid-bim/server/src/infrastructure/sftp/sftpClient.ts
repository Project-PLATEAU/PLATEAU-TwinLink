import SFTPClient from "ssh2-sftp-client";
import { ServerEnv } from "../../config";

const setUp = async (SFTP: SFTPClient) => {
  const config = ServerEnv.sftpConfig;
  await SFTP.connect(config);
};

export const getSftpClient = async () => {
  const sftp = new SFTPClient();
  await setUp(sftp);
  return sftp;
};
