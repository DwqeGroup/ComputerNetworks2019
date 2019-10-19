import java.io.File;
import java.io.FileInputStream;
import java.io.FileOutputStream;
import java.io.IOException;
import java.io.InputStream;
import java.io.OutputStream;
import java.net.SocketException;

import org.apache.commons.net.ftp.FTPClient;
import org.apache.commons.net.ftp.FTPFile;
import org.apache.commons.net.ftp.FTPReply;
import org.apache.log4j.Logger;

/**
 * �򵥲���FTP������ ,�˹�����֧�������ļ�������֧������Ŀ¼
 * �����Ҫ֧������Ŀ¼����Ҫ new String(path.getBytes("UTF-8"),"ISO-8859-1") ��Ŀ¼����ת��
 * @author WZH
 * 
 */
public class FTPUtils {

    private static Logger logger = Logger.getLogger(FTPUtils.class);

    /**
     * ��ȡFTPClient����
     * @param ftpHost ������IP
     * @param ftpPort �������˿ں�
     * @param ftpUserName �û���
     * @param ftpPassword ����
     * @return FTPClient
     */
    public FTPClient getFTPClient(String ftpHost, int ftpPort,
            String ftpUserName, String ftpPassword) {

        FTPClient ftp = null;
        try {
            ftp = new FTPClient();
            // ����FPT������,����IP���˿�
            ftp.connect(ftpHost, ftpPort);
            // �����û���������
            ftp.login(ftpUserName, ftpPassword);
            // �������ӳ�ʱʱ��,5000����
            ftp.setConnectTimeout(50000);
            // �������ı��뼯����ֹ��������
            ftp.setControlEncoding("UTF-8");
            if (!FTPReply.isPositiveCompletion(ftp.getReplyCode())) {
                logger.info("δ���ӵ�FTP���û������������");
                ftp.disconnect();
            } else {
                logger.info("FTP���ӳɹ�");
                //System.out.println("���ӳɹ�");
            }

        } catch (SocketException e) {
            e.printStackTrace();
            logger.info("FTP��IP��ַ���ܴ�������ȷ����");
        } catch (IOException e) {
            e.printStackTrace();
            logger.info("FTP�Ķ˿ڴ���,����ȷ����");
        }
        return ftp;
    }
    
    /**
     * �ر�FTP����
     * @param ftp
     * @return
     */
    public boolean closeFTP(FTPClient ftp){
        
        try {
            ftp.logout();
        } catch (Exception e) {
            logger.error("FTP�ر�ʧ��");
        }finally{
            if (ftp.isConnected()) {
                try {
                    ftp.disconnect();
                } catch (IOException ioe) {
                    logger.error("FTP�ر�ʧ��");
                }
            }
        }
        
        return false;
        
    }
    
    
    /**
     * ����FTP��ָ���ļ�
     * @param ftp FTPClient����
     * @param filePath FTP�ļ�·��
     * @param fileName �ļ���
     * @param downPath ���ر����Ŀ¼
     * @return
     */
    public boolean downLoadFTP(FTPClient ftp, String filePath, String fileName,
            String downPath) {
        // Ĭ��ʧ��
        boolean flag = false;

        try {
            // ��ת���ļ�Ŀ¼
            ftp.changeWorkingDirectory(filePath);
            // ��ȡĿ¼���ļ�����
            ftp.enterLocalPassiveMode();
            FTPFile[] files = ftp.listFiles();
            //System.out.println("start dowloading");
            for (FTPFile file : files) {
                // ȡ��ָ���ļ�������
                if (file.getName().equals(fileName)) {
                    File downFile = new File(downPath + File.separator
                            + file.getName());
                    System.out.println(downPath + File.separator
                            + file.getName());
                    OutputStream out = new FileOutputStream(downFile);
                    // ������������ļ�,��Ҫ���ñ��뼯����Ȼ���ܳ����ļ�Ϊ�յ����
                    flag = ftp.retrieveFile(new String(file.getName().getBytes("UTF-8"),"ISO-8859-1"), out);
                    // ���سɹ�ɾ���ļ�,����Ŀ����
                    // ftp.deleteFile(new String(fileName.getBytes("UTF-8"),"ISO-8859-1"));
                    out.flush();
                    out.close();
                    if(flag){
                        logger.info("���سɹ�");
                    }else{
                        logger.error("����ʧ��");
                    }
                }
            }
            //System.out.println("downloading end");

        } catch (Exception e) {
            logger.error("����ʧ��");
        } 

        return flag;
    }

    /**
     * FTP�ļ��ϴ�������
     * @param ftp
     * @param filePath
     * @param ftpPath
     * @return
     */
    public boolean uploadFile(FTPClient ftp,String filePath,String ftpPath){
        boolean flag = false;
        InputStream in = null;
        try {
         // ����PassiveMode����  
            ftp.enterLocalPassiveMode(); 
            //���ö����ƴ��䣬ʹ��BINARY_FILE_TYPE��ASC��������ļ���
            ftp.setFileType(FTPClient.BINARY_FILE_TYPE);
            //�ж�FPTĿ���ļ���ʱ����ڲ������򴴽�
            if(!ftp.changeWorkingDirectory(ftpPath)){
                ftp.makeDirectory(ftpPath);
            }
            //��תĿ��Ŀ¼
            ftp.changeWorkingDirectory(ftpPath);
            
            //�ϴ��ļ�
            File file = new File(filePath);
            in = new FileInputStream(file);
            String tempName = ftpPath+File.separator+file.getName();
            //System.out.println(ftpPath+File.separator+file.getName());
            flag = ftp.storeFile(new String (tempName.getBytes("UTF-8"),"ISO-8859-1"),in);
            if(flag){
                logger.info("�ϴ��ɹ�");
            }else{
                logger.error("�ϴ�ʧ��");
            }
        } catch (Exception e) {
            e.printStackTrace();
            logger.error("�ϴ�ʧ��");
        }finally{
            try {
                in.close();
            } catch (IOException e) {
                // TODO Auto-generated catch block
                e.printStackTrace();
            }
        }
        return flag;
    }   
    
    public static void main(String[] args) {
        FTPUtils test = new FTPUtils();
        FTPClient ftp = test.getFTPClient("192.168.43.66", 21, "123","123");
        //test.downLoadFTP(ftp, "/file", "���.jpg", "C:\\����");
        //test.copyFile(ftp, "/file", "/txt/temp", "���.txt");
        //test.uploadFile(ftp, "C:\\����\\���.jpg", "/");
        //test.moveFile(ftp, "/file", "/txt/temp");
        //test.deleteByFolder(ftp, "/txt");
        test.downLoadFTP(ftp, "/","test.txt", "C:\\Users\\tc\\Downloads");
        test.uploadFile(ftp, "C:\\Users\\tc\\Downloads\\upload_test.txt", "/");
        test.closeFTP(ftp);
        System.exit(0);
    }
}