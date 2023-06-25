//�Ƹ翪Դ��ת������������
//����DengFOC�ϲ��Թ�
//PWM������Ŷ���
int pwmA = 32;
int pwmB = 33;
int pwmC = 25;

//��ʼ��������������
#define _constrain(amt,low,high) ((amt)<(low)?(low):((amt)>(high)?(high):(amt)))
//�궨��ʵ�ֵ�һ��Լ������,��������һ��ֵ�ķ�Χ��
//������˵���ú궨�������Ϊ _constrain�������������� amt��low �� high���ֱ��ʾҪ���Ƶ�ֵ����Сֵ�����ֵ���ú궨���ʵ��ʹ������Ԫ����������� amt �Ƿ�С�� low ����� high���������е�������Сֵ�����߷���ԭֵ��
//���仰˵����� amt С�� low���򷵻� low����� amt ���� high���򷵻� high�����򷵻� amt��������_constrain(amt, low, high) �ͻὫ amt Լ���� [low, high] �ķ�Χ�ڡ�
float voltage_power_supply=12.6;
float shaft_angle=0,open_loop_timestamp=0;
float zero_electric_angle=0,Ualpha,Ubeta=0,Ua=0,Ub=0,Uc=0,dc_a=0,dc_b=0,dc_c=0;


void setup() 
{
  // put your setup code here, to run once:
  Serial.begin(115200);
  //PWM����
  pinMode(pwmA, OUTPUT);
  pinMode(pwmB, OUTPUT);
  pinMode(pwmC, OUTPUT);
  ledcAttachPin(pwmA, 0);
  ledcAttachPin(pwmB, 1);
  ledcAttachPin(pwmC, 2);
  ledcSetup(0, 30000, 8);  //pwmƵ��, Ƶ��, ����
  ledcSetup(1, 30000, 8);  //pwmƵ��, Ƶ��, ����
  ledcSetup(2, 30000, 8);  //pwmƵ��, Ƶ��, ����
  Serial.println("���PWM��ʼ������");
  delay(3000);

}

// ��Ƕ����
float _electricalAngle(float shaft_angle, int pole_pairs) 
{
  return (shaft_angle * pole_pairs);
}

// ��һ���Ƕȵ� [0,2PI]
float _normalizeAngle(float angle)
{
  float a = fmod(angle, 2*PI);   //ȡ������������ڹ�һ�����г�����ֵ�������֪
  return a >= 0 ? a : (a + 2*PI);  
  //��Ŀ���������ʽ��condition ? expr1 : expr2 
  //���У�condition ��Ҫ��ֵ���������ʽ����������������򷵻� expr1 ��ֵ�����򷵻� expr2 ��ֵ�����Խ���Ŀ�������Ϊ if-else ���ļ���ʽ��
  //fmod �����������ķ����������ͬ����ˣ��� angle ��ֵΪ����ʱ�������ķ��Ž��� _2PI �ķ����෴��Ҳ����˵����� angle ��ֵС�� 0 �� _2PI ��ֵΪ�������� fmod(angle, _2PI) ��������Ϊ������
  //���磬�� angle ��ֵΪ -PI/2��_2PI ��ֵΪ 2PI ʱ��fmod(angle, _2PI) ������һ������������������£�����ͨ������������������ _2PI �����Ƕȹ�һ���� [0, 2PI] �ķ�Χ�ڣ���ȷ���Ƕȵ�ֵʼ��Ϊ������
}


// ����PWM�����������
void setPwm(float Ua, float Ub, float Uc) 
{

  // ����ռ�ձ�
  // ����ռ�ձȴ�0��1
  dc_a = _constrain(Ua / voltage_power_supply, 0.0f , 1.0f );
  dc_b = _constrain(Ub / voltage_power_supply, 0.0f , 1.0f );
  dc_c = _constrain(Uc / voltage_power_supply, 0.0f , 1.0f );

  //д��PWM��PWM 0 1 2 ͨ��
  ledcWrite(0, dc_a*255);
  ledcWrite(1, dc_b*255);
  ledcWrite(2, dc_c*255);
}

void setPhaseVoltage(float Uq,float Ud, float angle_el) 
{
  angle_el = _normalizeAngle(angle_el + zero_electric_angle);
  // ������任
  Ualpha =  -Uq*sin(angle_el); 
  Ubeta =   Uq*cos(angle_el); 

  // ��������任
  Ua = Ualpha + voltage_power_supply/2;
  Ub = (sqrt(3)*Ubeta-Ualpha)/2 + voltage_power_supply/2;
  Uc = (-Ualpha-sqrt(3)*Ubeta)/2 + voltage_power_supply/2;
  setPwm(Ua,Ub,Uc);
}


//�����ٶȺ���
float velocityOpenloop(float target_velocity)
{
  unsigned long now_us = micros();  //��ȡ�ӿ���оƬ������΢���������ľ����� 4 ΢�롣 micros() ���ص���һ���޷��ų����ͣ�unsigned long����ֵ
  
  //���㵱ǰÿ��Loop������ʱ����
  float Ts = (now_us - open_loop_timestamp) * 1e-6f;

  //���� micros() �������ص�ʱ������ڴ�Լ 70 ����֮�����¿�ʼ����������70�������䵽0ʱ��TS������쳣�������Ҫ�������������ʱ����С�ڵ��������� 0.5 �룬��������Ϊһ����С��Ĭ��ֵ���� 1e-3f
  if(Ts <= 0 || Ts > 0.5f) Ts = 1e-3f;
  

  // ͨ������ʱ������Ŀ���ٶ���������Ҫת���Ļ�е�Ƕȣ��洢�� shaft_angle �����С��ڴ�֮ǰ������Ҫ����ǶȽ��й�һ������ȷ����ֵ�� 0 �� 2�� ֮�䡣
  shaft_angle = _normalizeAngle(shaft_angle + target_velocity*Ts);
  //��Ŀ���ٶ�Ϊ 10 rad/s Ϊ�������ʱ������ 1 �룬����ÿ��ѭ������Ҫ���� 10 * 1 = 10 ���ȵĽǶȱ仯��������ʹ���ת����Ŀ���ٶȡ�
  //���ʱ������ 0.1 �룬��ô��ÿ��ѭ������Ҫ���ӵĽǶȱ仯������ 10 * 0.1 = 1 ���ȣ�����ʵ����ͬ��Ŀ���ٶȡ���ˣ�������ת���Ƕ�ȡ����Ŀ���ٶȺ�ʱ�����ĳ˻���

  // ʹ����ǰ���õ�voltage_power_supply��1/3��ΪUqֵ�����ֵ��ֱ��Ӱ���������
  // ���ֻ������ΪUq = voltage_power_supply/2������ua,ub,uc�ᳬ�������ѹ�޷�
  float Uq = voltage_power_supply/3;
  
  setPhaseVoltage(Uq,  0, _electricalAngle(shaft_angle, 7));
  
  open_loop_timestamp = now_us;  //���ڼ�����һ��ʱ����

  return Uq;
}


void loop() {
  // put your main code here, to run repeatedly:
   velocityOpenloop(5);
}