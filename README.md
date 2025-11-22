# Libimage -次のメタバース-

Libertyとimagineを混ぜた造語です。

## このプロジェクトの目標

- インターネットの非匿名化から逃げたい人の救済の地にする
- 自由なゲームエンジンにして誰でも使いやすくする
- 誰でも遊びに来れる
- プライバシー第一
- LLM機能で次世代のゲーム体験を

## 使用(予定)技術

- C++ (主にグラフィックAPI関連)
- Rust (主要システムに)
- Python (AIの呼び出しなど)
- Lua (ゲーム内スクリプトとして)
- HTML/CSS + JS (Webサイトに)

## プラットフォームの理念

### 個人情報は大切である

今日のKnow Your Customer（顧客を知る）、以下KYCは、ネットの常識をひっくり返すほど怪しいものになっている。

### 主な理由

- 年齢確認に顔写真が必要
- パスポートや運転免許証を見せる必要がある
- 電話番号を登録する必要がある

これらが必要な理由は、表向きは未成年者の被害の防止や、犯罪の抑制であるが、どう考えても今後のAI開発(とくに人間の顔)の精度を上げるために情報を収集しているに違いないと私は考えている。

これらの情報が漏れたときのことを考えてみてほしい。顔、指紋、氏名などが漏洩した場合、それらを変更するのは困難である。そのため、私は年齢認証に別の手段を用いたいと考えている。

### 年齢認証に使用するための候補

- クレジットカード(顔写真や氏名よりはマシである。オンライン決済にも利用できるので、妥当な範囲内。)
- 知識テスト(AI時代にこれはあまり役に立たないかもしれない。難しすぎると、成人すらはじいてしまう。)
- 電話番号(適切にハッシュ化して登録すれば、サーバーが攻撃されても影響が少ない。少なくとも12歳以上であるかの目安にはなる。)

### 遊びやすく、そして作りやすく

皆さんはなにかツールを使おうと思ったときに、どうすればいいのかがわからず挫折したことはないだろうか？なぜこれが起きるかといえば:

- 複雑で入り組んだUI
- チュートリアルすらわかりづらい
- 納得できない仕様(iMovieの字幕が自由な位置に置けないとか、アニメーションが無効にできないとか、そういうやつ。)

これらを解決するために、このようなデザイン哲学を実施する:

- チュートリアルなしでもわかりやすく
- 線形のUI(複雑な入れ子を避ける)
- ユーザーができるだけカスタマイズできるようにする

これによりユーザーは直感的に作業を行い、イメージを形にすることができる。数年前のRobloxがそうであったように。

### 技術を持たない人にも力を

人間には得意分野があれば不得意な分野もあります。とくにメタバースで考えれば:

- プログラミング
- 3Dモデリング
- イラスト
- 作曲
- アニメーション
- シナリオ
- テクスチャ
- アーキテクチャ

など、少し考えるだけでもいろいろな分野があるのです。

なので、基本的なツールを与えることで、これら全般をやりやすくするIDEを目指したいと考えています。

---

## 具体的な設計

### 物理エンジン

あまりこの分野には詳しくないので、使えそうな公式をまとめておく。

たとえ公式があっても、最適化は困難だが。

### 重力

$F=G*(Mm/r^2)$

### 描画エンジン

どのOSでも使えるOpenGLでとりあえずつくる。そのあとにMetalとかで最適化すれば良い。

### 高速逆平方根

```c
float Q_rsqrt( float number )
{
    long i;
    float x2, y;
    const float threehalfs = 1.5F;

    x2 = number * 0.5F;
    y = number;
    i = * ( long * ) &y;    // evil floating point bit level hacking
    i = 0x5f3759df - ( i >> 1 );              // what the fuck?
    y = * (      * ) &i;
    y = y * (threehalfs - ( x2 * y * y ) );   // 1st iteration
//  y = y * (threehalfs - ( x2 * y * y ) );   // 2nd iteration,
                                           // this can be removed

    return y;
}
```

これを使う機会があればやってみたい。

### 3Dエンジン

### Pythonのコード

```python
import pygame
import math

minX, maxX = 0, 800
minY, maxY = 0, 600

colors = {
    "black": (0, 0, 0),
    "white": (255, 255, 255)
}

workspace = []

class Vector2:
    def __init__(self, x, y) -> None:
        self.x = x
        self.y = y

class Vector3:
    def __init__(self, x, y, z) -> None:
        self.x = x
        self.y = y
        self.z = z

    def __add__(self, other):
        if not isinstance(other, Vector3):
            error(TypeError, "Only Vector3s can be added")
        return Vector3(self.x + other.x, self.y + other.y, self.z + other.z)
    
    def __sub__(self, other):
        if not isinstance(other, Vector3):
            error(TypeError, "Only Vector3s can be subtracted")
        return Vector3(self.x - other.x, self.y - other.y, self.z - other.z)

    def __mul__(self, scalar):
        return Vector3(self.x * scalar, self.y * scalar, self.z * scalar)

    def __truediv__(self, scalar):
        return Vector3(self.x / scalar, self.y / scalar, self.z / scalar)

    def __repr__(self):
        return f"Vector3({self.x}, {self.y}, {self.z})"

class Cube:
    def __init__(self, size:Vector3, pos:Vector3, color:Vector3) -> None:
        self.sizeX = size.x
        self.sizeY = size.y
        self.sizeZ = size.z

        self.x = pos.x
        self.y = pos.y
        self.z = pos.z
        self.color = color
    def new(self):
        workspace.append(self)

cam_f = 200
camRotation = Vector3(0, 0, 0)
camPos = Vector3(0, 0, -100)
Origin = Vector2(maxX/2, maxY/2)

def warn(message):
    text = '\033[33m' + message + '\033[0m' #黄色で出力
    print(text)

def error(reason: type[BaseException], message: str):
    text = f'\033[31m{reason.__name__}: {message}\033[0m' #赤色
    print(text)
    raise reason

def toTuple(v):
    if isinstance(v, Vector2):
        return (v.x, v.y)
    elif isinstance(v, Vector3):
        return (v.x, v.y, v.z)
    else:
        warn(f'''Couldn't convert the instance: {v}''')
        return ()
    
def toColor(v):
    if isinstance(v, Vector2):
        return (v.x, v.y)
    elif isinstance(v, Vector3):
        r = int(max(0, min(255, v.x)))
        g = int(max(0, min(255, v.y)))
        b = int(max(0, min(255, v.z))) # zはRGBのBとして扱われている
        return (r, g, b)
    else:
        warn(f'''Couldn't convert the instance: {v}''')
        return ()

sin, cos = math.sin, math.cos

def rotateX(v3: Vector3, degree):
    rad = math.radians(degree)
    y2 = v3.y * cos(rad) - v3.z * sin(rad)
    z2 = v3.y * sin(rad) + v3.z * cos(rad)
    return Vector3(v3.x, y2, z2)

def rotateY(v3: Vector3, degree):
    rad = math.radians(degree)
    cos_r = math.cos(rad)
    sin_r = math.sin(rad)

    x2 = v3.x * cos_r + v3.z * sin_r
    z2 = -v3.x * sin_r + v3.z * cos_r
    return Vector3(x2, v3.y, z2)
def rotateZ(v3: Vector3, degree):
    rad = math.radians(degree)
    x2 = v3.x * cos(rad) - v3.y * sin(rad)
    y2 = v3.x * sin(rad) + v3.y * cos(rad)
    return Vector3(x2, y2, v3.z)

def get_camera_directions(camRotation):
    yaw = math.radians(camRotation.y)
    pitch = math.radians(camRotation.x)
    roll = math.radians(camRotation.z)

    forward = Vector3(
        math.sin(yaw) * math.cos(pitch),
        -math.sin(pitch),
        math.cos(yaw) * math.cos(pitch)
    )

    right = Vector3(
        math.cos(yaw),
        0,
        -math.sin(yaw)
    )

    # rollを考慮したい場合は少し複雑になるけど、
    # rollを無視するなら単純にこう
    up = Vector3(
        -math.sin(yaw) * math.sin(pitch),
        math.cos(pitch),
        -math.cos(yaw) * math.sin(pitch)
    )

    return forward, right, up

def project(v3: Vector3, f, origin: Vector2):
    """
    ワールド座標 v3 をカメラ位置 camPos とカメラ回転 camRotation を基準に投影
    """
    # 1. カメラ座標系に変換（カメラを原点に移動）
    rel = v3 - camPos

    # 2. カメラ回転をワールド→カメラ座標系に逆向きで適用
    #    Yaw (y軸回転), Pitch (x軸回転), Roll (z軸回転) の順
    rel = rotateY(rel, -camRotation.y)
    rel = rotateX(rel, -camRotation.x)
    rel = rotateZ(rel, -camRotation.z)

    # 3. 投影（透視投影）
    if rel.z <= 0.01:  # 0除算防止
        rel.z = 0.01
    x2d = (rel.x * f) / rel.z
    y2d = (rel.y * f) / rel.z

    # 4. 画面原点に平行移動
    return (origin.x + x2d, origin.y - y2d)

def render():
    part:Cube

    # オブジェクトの「カメラから見た」Z座標を計算する内部関数
    def get_camera_space_z(cube):
        center_pos = Vector3(cube.x, cube.y, cube.z)
        # 1. カメラからの相対位置
        rel = center_pos - camPos
        # 2. カメラの回転を適用
        rel = rotateY(rel, -camRotation.y)
        rel = rotateX(rel, -camRotation.x)
        rel = rotateZ(rel, -camRotation.z)
        # 3. カメラ座標系でのZ値を返す
        return rel.z

    # カメラから遠い順 (zが大きい順) にソート
    sorted_workspace = sorted(workspace, key=get_camera_space_z, reverse=True)
    for part in sorted_workspace:
        midSX = part.sizeX / 2
        midSY = part.sizeY / 2
        midSZ = part.sizeZ / 2

        #前面
        A = Vector3(part.x - midSX, part.y + midSY, part.z - midSZ)
        B = Vector3(part.x - midSX, part.y - midSY, part.z - midSZ)
        C = Vector3(part.x + midSX, part.y - midSY, part.z - midSZ)
        D = Vector3(part.x + midSX, part.y + midSY, part.z - midSZ)

        #背面
        E = Vector3(part.x - midSX, part.y + midSY, part.z + midSZ)
        F = Vector3(part.x - midSX, part.y - midSY, part.z + midSZ)
        G = Vector3(part.x + midSX, part.y - midSY, part.z + midSZ)
        H = Vector3(part.x + midSX, part.y + midSY, part.z + midSZ)

        pfront = [
            project(A, cam_f, Origin),
            project(B, cam_f, Origin),
            project(C, cam_f, Origin),
            project(D, cam_f, Origin),
        ]

        pback = [
            project(E, cam_f, Origin),
            project(F, cam_f, Origin),
            project(G, cam_f, Origin),
            project(H, cam_f, Origin),
        ]

        lines = []
        for i in range(4):
            lines.append((pfront[i], pback[i]))

        pygame.draw.polygon(screen, toColor(part.color * 0.5), pback, 1)
        pygame.draw.polygon(screen, toColor(part.color), pfront, 1)
        for line in lines:
            pygame.draw.line(screen, toColor(part.color * 0.75), line[0], line[1])

pygame.init()
screen = pygame.display.set_mode(
    (maxX, maxY),
    pygame.RESIZABLE | pygame.SCALED,
    vsync=1
)

pygame.display.set_caption("3D engine")
clock = pygame.time.Clock()

running = True
part = Cube(Vector3(100, 100, 100), Vector3(0, 0, -100), Vector3(0, 100, 200))
[part.new](http://part.new)()

part2 = Cube(Vector3(50, 50, 50), Vector3(100, 100, 0), Vector3(0, 150, 100))
[part2.new](http://part2.new)()

part3 = Cube(Vector3(1000, 25, 1000), Vector3(0, -25, 0), Vector3(0, 255, 0))
[part3.new](http://part3.new)()

speed = 1
while running:
    screen.fill(colors["white"])
    for event in pygame.event.get():
        if event.type == pygame.QUIT:
            running = False
    keys = pygame.key.get_pressed()

    forward, right, up = get_camera_directions(camRotation)
    if keys[pygame.K_q]:
        camPos = camPos - up * speed
    if keys[pygame.K_e]:
        camPos = camPos + up * speed
    if keys[pygame.K_w]:
        camPos = camPos + forward * speed
    if keys[pygame.K_s]:
        camPos = camPos - forward * speed
    if keys[pygame.K_a]:
        camPos = camPos - right * speed
    if keys[pygame.K_d]:
        camPos = camPos + right * speed
    
    if keys[pygame.K_UP]:
        camRotation = camRotation + Vector3(-1, 0, 0)
    if keys[pygame.K_DOWN]:
        camRotation = camRotation + Vector3(1, 0, 0)
    if keys[pygame.K_LEFT]:
        camRotation = camRotation + Vector3(0, -1, 0)
    if keys[pygame.K_RIGHT]:
        camRotation = camRotation + Vector3(0, 1, 0)
    render()
    pygame.display.flip()
    clock.tick(60)
pygame.quit()
```

Pygameでは限界があった(CPUは塗り潰しを行うととてつもなく重くなる)。

グラフィックライブラリの必要性を感じ、計画を打ち切りにしたが、この経験を活かしてこのプロジェクトの雛形とする。

### LLMの呼び出し

誰かいい感じのローカルモデルがあれば教えてほしい。どうやってダウンロードするかもしらないし、呼び出し方もしらない。

### 最初に作る予定のインスタンス

- BasePart(Robloxから引用)
    - Part
- Script(Luaで動作)
- Player(ユーザー操作などを扱う)
- Model(集合として、複数Partを操作しやすく)
- Folder(整頓用)

### デフォルト操作

- WASDで移動(伝統的)
- 右クリックのドラッグで視点移動
- 左クリックでインタラクト
- スペースでジャンプ

---# Libimage
