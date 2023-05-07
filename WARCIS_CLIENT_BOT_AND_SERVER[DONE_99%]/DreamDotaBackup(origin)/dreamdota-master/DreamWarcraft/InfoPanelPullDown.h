#ifndef INFOPANEL_PULLDOWN_H_
#define INFOPANEL_PULLDOWN_H_

class UISimpleFrame;
class UISimpleButton;
class UISimpleTexture;
class UILayoutFrame;
class Label;
class Observer;

namespace InfoPanel {
	class PullDown;

	typedef void (*PullDownCallback) (PullDown*, uint32_t);

	class PullDown {
		UISimpleFrame*		currentItemFrame;	//��ǰ��ʾframe
		UISimpleTexture*	pullDownArrow;		//��ʾ���������Ǽ�ͷ
		UISimpleButton*		currentItemClicker;	//��׽����ؼ�
		Observer*			currentItemClickerOb;
		Label*				currentItemText;	//��ǰ��ʾ����

		UISimpleFrame*		pullDownFrame;		//������ʾframe
		bool				pullDownFrameIsHidden;
		std::vector<UISimpleButton*>	itemClickerSet;	//��׽ÿһ�����ؼ�
		std::vector<Observer*>			itemClickerSetOb;
		std::vector<Label*>				itemTextSet;	//ÿһ������

		uint32_t			currentItemIndex;
		float				itemTextHeight;

		PullDownCallback	callback;

	public:
		PullDown(
			UISimpleFrame*		parent,
			float				inWidth,
			float				inItemHeight,
			uint32_t			inItemCount,
			PullDownCallback	callback = NULL
		);

		~PullDown();

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

		void expand (bool flag);
		bool isExpanded ();
		void toggle ();
		void setIndex (uint32_t index);
		void toggleIndex (uint32_t index);
		uint32_t getIndex () const;
		void setText (uint32_t index, char *format, ...);
		void setColor (uint32_t index, float r, float g, float b, float a);
		void show(bool flag);

		static void Init();
	};

}

#endif