#ifndef INFOPANEL_ROW_H_
#define INFOPANEL_ROW_H_

class UISimpleFrame;
class UISimpleTexture;
class UISimpleFontString;
class UILayoutFrame;
class Label;

namespace InfoPanel {
	//keyΪ��Ŀ��(0-based)
	typedef std::map< uint32_t, UISimpleFontString* > TextContentType;

	//����һ�У�����˹̶�Ϊ�������ͼ�� + ��Һ�(1-based)
	//��ʽ��һ��ϸ�߿��ޱ�����UISimpleFrame
	//��С��ָ�����¿�ȣ�ָ����Ŀ��ȣ����ݾ�����λ��
	//�ɰ��� obj[0], obj[1]����access��Ŀ��[0]Ϊ��һ����Ŀ
	class Row {
		uint32_t					itemCount;			//��Ŀ����

		UISimpleFrame*				frame;				//������λ�ͱ����Ļ�����
		UISimpleFrame*				playerIconFrame;
		float						height;
		float						itemWidth;
		float						width; //��Ϊ0ʱǿ��Ϊ���
		bool						hidden;

		UISimpleTexture*			playerColorBg;		//�����ɫ
		UISimpleTexture*			playerRaceIcon;		//�������ͼ��
		Label*						playerIdLabel;
		int							playerId;			//��Һ�(0-based)	 TODO���������
		bool						playerWantUpdate;	//�Ƿ���update�и���

		TextContentType				contentSimpleFontStrings;
		std::map<uint32_t, std::string>	contentText;

	public:

		Row(UISimpleFrame *parent, float inHeight, float inItemWidth, uint32_t inItemCount);
		~Row();

		void	setPlayerId (int inPlayerId);
		void	setText (uint32_t index, char *format, ...);
		void	setWidth (float width);

		UILayoutFrame *getFrame();
		float	getHeight() {return height;}

		void	setRelativePosition(
			uint32_t originPos, 
			UILayoutFrame* target, 
			uint32_t toPos, 
			float relativeX, 
			float relativeY
		);

		void	setAbsolutePosition(
			uint32_t originPos, 
			float absoluteX, 
			float absoluteY
		);

		void	show(bool flag = true);
		bool	isHidden();
		void	update();						//�����������ݸ���
	};

}

#endif