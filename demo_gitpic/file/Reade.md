# �ļ�������غ����ܽ�
## scandir
���庯����int  scandir(const char *dir, struct dirent **namelist, nt (*select)(const  struct  dirent *), nt (*compar)(const struct dirent **, const struct dirent**));
����˵����scandir()��ɨ�����dirָ����Ŀ¼�ļ������ɲ���selectָ���ĺ�������ѡĿ¼�ṹ������namelist�����У�
		  ����ٵ��ò���comparָ���ĺ���������namelist�����е�Ŀ¼���ݡ�
		  ÿ�δ�Ŀ¼�ļ��ж�ȡһ��Ŀ¼�ṹ��㽫�˽ṹ��������select��ָ�ĺ����� 
		  select����������Ҫ����Ŀ¼�ṹ���Ƶ�namelist����ͷ���0����selectΪ��ָ�������ѡ�����е�Ŀ¼�ṹ��
		  scandir()�����qsort()���������ݣ�����compar��Ϊqsort()�Ĳ���������Ҫ����Ŀ¼������ĸ���ʹ��alphasort() 
		  �ṹdirent������ο�readdir()
����ֵ  ���ɹ��򷵻ظ��Ƶ�namelist�����е����ݽṹ��Ŀ���д������򷵻�-1
