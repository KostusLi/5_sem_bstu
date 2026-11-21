using ASPA0010_1.DTO;
using Microsoft.AspNetCore.Authorization;
using Microsoft.AspNetCore.Http;
using Microsoft.AspNetCore.Mvc;

namespace ASPA0010_1.Controllers
{
    [Route("api/Results")]
    [ApiController]
    public class ResultsController : ControllerBase
    {

        private readonly ResultsCollection.Results _results;
        private readonly ResultsAuthenticate.Authenticate _auth;

        public ResultsController(ResultsCollection.Results res, ResultsAuthenticate.Authenticate auth)
        {
            _results = res;
            _auth = auth;
        }

        [HttpPost("SignIn")]
        public async Task<IActionResult> SignIn([FromBody] LoginModel model)
        {
            if (model == null || string.IsNullOrEmpty(model.login) || string.IsNullOrEmpty(model.password))
                return BadRequest();

            bool isSuccess = await _auth.SignInAsync(model.login, model.password);

            if (isSuccess)
                return Ok("Cookie-токен установлен");

            return NotFound();
        }

        [HttpGet("SignOut")]
        [Authorize]
        public async Task<IActionResult> SignOut()
        {
            string shitJwt = await _auth.SignOutAsync();
            return Ok(new { token = shitJwt });
        }

        [HttpGet]
        [Authorize(Roles ="READER")]
        public IActionResult GetResults()
        {
            var ris = _results.GetAll();

            if (ris.Count!=0)
            {
                return Ok(ris);
            }
            else
            {
                return NoContent();
            }
            
        }

        [HttpGet("{k:int}")]
        [Authorize(Roles = "READER")]
        public IActionResult GetDyId(int k)
        {
            var ris = _results.GetById(k);

            if (ris != null)
            {
                return Ok(ris);
            }
            else
            {
                return NotFound();
            }
        }

        [HttpPost]
        [Authorize(Roles = "WRITER")]
        public IActionResult AddResults([FromBody] ValueModel model)
        {
            if(model is null || string.IsNullOrWhiteSpace(model.Value))
            {
                return BadRequest();
            }

            var ris = _results.Add(model.Value);

            return Created($"/api/Results/{ris.Key}", ris);
        }

        [HttpPut("{k:int}")]
        [Authorize(Roles = "WRITER")]
        public IActionResult ChangeResult(int k, [FromBody] ValueModel model)
        {
            if(model is null || string.IsNullOrWhiteSpace(model.Value))
            {
                return BadRequest();
            }

            var check = _results.Change(k, model.Value);

            if(check)
            {
                return Ok(new KeyValuePair<int, string>(k, model.Value!));
            }

            return NotFound();
        }

        [HttpDelete("{k:int}")]
        [Authorize(Roles = "WRITER")]
        public IActionResult DeleteResult(int k)
        {
            var removeData = _results.DeleteById(k);
            return (removeData is not null) ? Ok(removeData) : NotFound();
        }

    }
}
